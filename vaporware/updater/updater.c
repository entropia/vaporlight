#define _DEFAULT_SOURCE

#include <errno.h>
#include <error.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// CRC computation a la STM32
// see http://www.st.com/st-web-ui/static/active/en/resource/technical/document/application_note/DM00068118.pdf
const uint32_t initial_crc_state = 0xffffffff;
uint32_t crc_add(uint32_t crc_state, uint32_t input_data) {
	static const uint32_t polynomial = 0x04c11db7;

	crc_state ^= input_data;

	for (size_t i = 0; i < sizeof(input_data) * 8; i++) {
		uint32_t msb = crc_state & 0x80000000;

		crc_state <<= 1;
		if (msb) {
			crc_state ^= polynomial;
		}
	}

	return crc_state;
}

/*
 * Read IHEX file at file into buf. buf starts at addr_offset, and is buflen long.
 */
int read_ihex_from_file(FILE *file, unsigned char *buf, uint32_t addr_offset, size_t buflen) {
	char *line = NULL;
	int lineno = 0;
	size_t len = 0;

	// Upper 16 bits of the current data pointer.
	// (already shifted into place)
	uint32_t ihex_upper_addr = 0;

	uint8_t *bytes = NULL;

	int ret = 0;

	memset(buf, 0, buflen);

	while (true) {
		int err = getline(&line, &len, file);
		lineno++;

		if (err == -1) {
			if (feof(file)) {
				fprintf(stderr, "Warning: Missing EOF record\n");
				goto out;
			} else {
				goto error_out;
			}
		}

		if (line[0] != ':') {
			fprintf(stderr, "Ignoring line %d: %s", lineno, line);
			continue;
		}

		/*
		 * Read bytes
		 */

		uint8_t data_len;
		if (sscanf(line + 1, "%02hhX", &data_len) != 1) {
			fprintf(stderr, "No length found at line %d: %s", lineno, line);
		}

		unsigned int bytes_len = data_len + 1 + 2 + 1 + 1; // length, addr opcode, checksum;

		unsigned int expect_length = 2 * bytes_len + 1; // +1 for the ':'
		if (strlen(line) < expect_length) {
			fprintf(stderr, "Line %d (%s) ends short\n", lineno, line);
			goto error_out;
		}

		uint8_t *bytes = malloc(bytes_len);
		bytes[0] = data_len;
		for (unsigned int i = 1, col = 3; i < bytes_len; i++, col += 2) {
			uint8_t b;
			if (sscanf(line + col, "%02hhX", &b) != 1) {
				fprintf(stderr, "Parse error at (%d:%d): %s", lineno, col, line);
			}
			bytes[i] = b;
		}

		/*
		 * Compute checksum
		 */

		uint8_t checksum = 0;
		for (unsigned int i = 0; i < bytes_len; i++) {
			checksum += bytes[i];
		}
		if (checksum != 0) {
			fprintf(stderr, "Checksum mismatch in line %d: %s", lineno, line);
			goto error_out;
		}

		/*
		 * Interpret
		 */

		/*
		 * Format of an IHEX line (in bytes):
		 *  /- Number of data bytes
		 *  |  /- Address (2 bytes big endian)
		 *  |  |     /- Opcode (see below)
		 *  |  |     |  /- Data
		 *  |  |     |  |          /- Checksum (Sum of all bytes including this % 256 == 0)
		 *  v  v     v  v          v
		 * [l, a, a, o, d, ..., d, c]
		 */

		enum {
			HEXOP_DATA          = 0x00, // Data
			HEXOP_EOF           = 0x01, // End of file
			HEXOP_SEGMENT_ADDR  = 0x02, // Set segment base address (not supported)
			HEXOP_SEGMENT_START = 0x03, // Set initial IP for segmented addressing (not supported)
			HEXOP_LINEAR_ADDR   = 0x04, // Set upper 16 bits of linear address
			HEXOP_LINEAR_START  = 0x05  // Set initial IP for linear addressing (ignored)
		};

		uint8_t opcode = bytes[3];
		uint16_t addr = (bytes[1] << 8) + bytes[2];

		switch(opcode) {
		case HEXOP_DATA: ;
			uint32_t offset = ihex_upper_addr;
			offset -= addr_offset;
			offset += addr;

			if (data_len == 0) {
				fprintf(stderr, "Warning: Zero-length Data record in line %d: %s\n", lineno, line);
			} else {
				if (offset + data_len - 1 > buflen) {
					fprintf(stderr, "Address out of bounds in line %d: %s\n", lineno, line);
					goto error_out;
				}

				memcpy(buf + offset, bytes + 4, data_len);
			}
			break;

		case HEXOP_EOF:
			goto out;
			break;

		case HEXOP_SEGMENT_ADDR:
		case HEXOP_SEGMENT_START:
			error(2, 0, "Segmented addressing not supported");
			break;

		case HEXOP_LINEAR_ADDR:
			if (data_len != 2) {
				fprintf(stderr, "Cannot parse Extended Linear Address record in line %d: %s\n", lineno, line);
				goto error_out;
			}

			ihex_upper_addr = (bytes[4] << 24) | (bytes[5] << 16);
			break;

		case HEXOP_LINEAR_START:
			fprintf(stderr, "Ignoring initial IP setting\n");
			break;
		}

		free(bytes);
	}

error_out:
	ret = 1;
out:
	free(bytes);
	free(line);
	return ret;
}

bool should_retry() {
	char *line = NULL;
	size_t len;
	int err = getline(&line, &len, stdin);

	if (err == -1) {
		error(2, errno, "I/O error");
	}

	bool ret = strcmp(line, "r\n") == 0;

	free(line);

	return ret;
}

#define TTY_OUT(data) if (write_slowly((data), sizeof(data), ttyfile) != 0) return 1

unsigned int write_slowly(uint8_t *data, size_t size, FILE *file) {
	for (size_t i = 0; i < size; i++) {
		int ret = fputc(data[i], file);
		if (ret == EOF) {
			return 1;
		}
		ret = fflush(file);
		if (ret == EOF) {
			return 1;
		}
		usleep(1000);
	}

	return 0;
}

int flash_buffer(uint8_t *buf, size_t buflen, size_t pagesize, uint8_t module_addr, FILE *ttyfile) {
	printf("When asked, press <return> to continue, or r<return> to retry\n");

	do {
		uint8_t flash_cmd[] = {
			0x55,
			module_addr,
			0xfe,
		};
		TTY_OUT(flash_cmd);
		printf("Sent update command. Retry? "); fflush(stdout);
	} while(should_retry());

	size_t page_count = buflen / pagesize;
	bool retry = false;

	do {
		for (size_t i = 0; i < page_count; i++) {
			uint8_t erase_cmd[] = {
				0x55,
				0xf0,
				(uint8_t) i
			};
			TTY_OUT(erase_cmd);
			usleep(100000);

			uint32_t crc_state = initial_crc_state;

			for (size_t j = 0; j < pagesize; j+=4) {
				uint32_t all = (buf[i * pagesize + j + 0] << 0) |
					(buf[i * pagesize + j + 1] << 8) |
					(buf[i * pagesize + j + 2] << 16) |
					(buf[i * pagesize + j + 3] << 24);
				crc_state = crc_add(crc_state, all);

				uint8_t write_cmd[] = {
					0x55,
					0xf1,
					buf[i * pagesize + j + 0],
					buf[i * pagesize + j + 1],
					buf[i * pagesize + j + 2],
					buf[i * pagesize + j + 3]
				};
				TTY_OUT(write_cmd);
			}

			uint8_t crc_cmd[] = {
				0x55,
				0xf2,
				crc_state         & 0xff,
				(crc_state >> 8)  & 0xff,
				(crc_state >> 16) & 0xff,
				(crc_state >> 24) & 0xff
			};
			TTY_OUT(crc_cmd);

			printf("Flashed page %lu of %lu\n", i, page_count - 1);

		}

		printf("Retry? "); fflush(stdout);
		retry = should_retry();
		if (retry) {
			uint8_t retry_cmd[] = {
				0x55,
				0xf3
			};
			TTY_OUT(retry_cmd);
		}
	} while (retry);


	do {
		uint8_t reset_cmd[] = {
			0x55,
			0xf4
		};
		TTY_OUT(reset_cmd);
		printf("Reset board. Retry?"); fflush(stdout);
	} while (should_retry());

	printf("Done\n");
	return 0;
}

int main(int argc, char **argv) {
	int err;

	uint32_t flash_begin = 0x08000000;
	uint32_t flash_end   = flash_begin + (15 * 1024);
	int      pagesize    = 1024;

	if (argc != 4) {
		error(1, 0, "Expected arguments: IHEX-file tty-file module-address");
	}

	char *inpath = argv[1];
	char *tty_path = argv[2];
	char *module_addr_string = argv[3];

	uint8_t module_addr;

	/* read module address */ {
		unsigned int raw_addr;
		int chars_read;
		if ((sscanf(module_addr_string, "%x%n", &raw_addr, &chars_read) < 1) ||
		    ((size_t)chars_read < strlen(module_addr_string)) ||
		    (raw_addr > 0xff)) {
			error(1, errno, "Could not parse module address");
		}
		module_addr = raw_addr;
	}

	if ((flash_end - flash_begin) % pagesize != 0) {
		fprintf(stderr, "Warning: Memory area is not divisible by page size. Rounding up.\n");
		int mod = (flash_end - flash_begin) % pagesize;
		flash_end += pagesize - mod;
	}

	FILE *infile = fopen(inpath, "r");
	if (infile == NULL) {
		error(2, errno, "Could not open HEX file");
	}

	size_t buflen = flash_end - flash_begin;
	unsigned char *buf = malloc(buflen);
	if (!buf) {
		error(2, errno, "Could not allocate buffer");
	}

	printf("Reading HEX file\n");
	err = read_ihex_from_file(infile, buf, flash_begin, buflen);
	if (err) {
		error(2, 0, "Could not parse HEX file");
	}

	// Not the best, but the simplest way to set our TTY
	// (no need to fool around with termios)
	size_t stty_length = strlen("stty -F  500000 raw") + strlen(tty_path) + 1;
	char *stty_invocation = malloc(stty_length);
	err = snprintf(stty_invocation, stty_length, "stty -F %s 500000 raw", tty_path);
	if (err >= (int)stty_length) {
		error(2, 0, "Internal error: Strings wrongly computed");
	}
	err = system(stty_invocation);
	if (err) {
		error(2, errno, "stty call failed");
	}

	FILE *ttyfile = fopen(tty_path, "wb");
	if (ttyfile == NULL) {
		error(2, errno, "Could not open TTY file\n");
	}

	if (setvbuf(ttyfile, NULL, _IONBF, 0)) {
		error(2, errno, "Cannot set TTY to unbuffered\n");
	}

	if (flash_buffer(buf, buflen, pagesize, module_addr, ttyfile)) {
		error(2, errno, "Error writing to TTY\n");
	}
}

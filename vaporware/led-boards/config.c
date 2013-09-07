#include "config.h"

#include "console.h"
#include "debug.h"
#include "flash.h"

#define REPEAT(value,length) { [0 ... (length - 1)] = value }

/*
 * Configuration values in RAM for access by other modules.
 */

config_entry_t config = {
	.my_address = 0x00fd,
	.heat_limit = REPEAT(0xffff, HEAT_SENSOR_LEN),
	.led_infos = {
		{
			// Example values for sRGB
			.color_matrix = {
				 2.409638554216868f,  -0.6693440428380186f, -0.321285140562249f,
				 -1.204819277108434f,  2.186523873270861f,   0.0495314591700134f,
				 -1.204819277108434f, -1.517179830432843f,   1.271753681392236f
			},
			.peak_Y = {
				10000,
				10000,
				10000
			},
			.channels = {
				0,
				1,
				2
			}
		},
		{
			// Example values for sRGB
			.color_matrix = {
				 2.409638554216868f,  -0.6693440428380186f, -0.321285140562249f,
				 -1.204819277108434f,  2.186523873270861f,   0.0495314591700134f,
				 -1.204819277108434f, -1.517179830432843f,   1.271753681392236f
			},
			.peak_Y = {
				10000,
				10000,
				10000
			},
			.channels = {
				3,
				4,
				5
			}
		},
		{
			// Example values for sRGB
			.color_matrix = {
				 2.409638554216868f,  -0.6693440428380186f, -0.321285140562249f,
				 -1.204819277108434f,  2.186523873270861f,   0.0495314591700134f,
				 -1.204819277108434f, -1.517179830432843f,   1.271753681392236f
			},
			.peak_Y = {
				10000,
				10000,
				10000
			},
			.channels = {
				6,
				7,
				8
			}
		},
		{
			// Example values for sRGB
			.color_matrix = {
				 2.409638554216868f,  -0.6693440428380186f, -0.321285140562249f,
				 -1.204819277108434f,  2.186523873270861f,   0.0495314591700134f,
				 -1.204819277108434f, -1.517179830432843f,   1.271753681392236f
			},
			.peak_Y = {
				10000,
				10000,
				10000
			},
			.channels = {
				9,
				10,
				11
			}
		},
		{
			// Example values for sRGB
			.color_matrix = {
				 2.409638554216868f,  -0.6693440428380186f, -0.321285140562249f,
				 -1.204819277108434f,  2.186523873270861f,   0.0495314591700134f,
				 -1.204819277108434f, -1.517179830432843f,   1.271753681392236f
			},
			.peak_Y = {
				10000,
				10000,
				10000
			},
			.channels = {
				12,
				13,
				14
			}
		}
	},
};

/*
 * The configuration page. This is laid out in the following way:
 * There is a number of configuration slots, used one after the other
 * for storing a configuration. The state of each slot is kept in
 * entry_status, where 0xffff (the flash default value) stands for a free
 * slot, 0x5555 for the slot currently in use and 0x0000 for an old slot.
 * When all slots are used, the config page is erased and the process
 * starts again.
 *
 * The entry count is derived in the following way:
 * Page size: 1024B
 * Entry size with status word: sizeof(config_entry_t) + sizeof(uint16_t)
 * Entry count = Page size / Entry size
 */
#define ENTRY_COUNT (FLASH_PAGE_SIZE * CONFIG_PAGES /             \
		     (sizeof(config_entry_t) + sizeof(uint16_t)))
typedef struct {
	uint16_t entry_status[ENTRY_COUNT];

	config_entry_t entries[ENTRY_COUNT];
} __attribute__ ((packed)) config_page_t;

config_page_t config_page __attribute__ ((section (".config"))) = {
	.entry_status = REPEAT(0xffff, ENTRY_COUNT),
	.entries = {
		[0 ... ENTRY_COUNT - 1] = {
			.my_address = 0xffff,
			.heat_limit = REPEAT(0xffff, HEAT_SENSOR_LEN),
			.led_infos = {
				[0 ... MODULE_LENGTH/3 - 1] = {
					.filler1 = REPEAT(0xff, 9 * sizeof(float)),
					.filler2 = REPEAT(0xff, 3 * sizeof(float)),
					.channels = REPEAT(0xff, 3)
				}
			},
			.padding = 0xff
		}
	}
};

/*
 * Loads the configuration stored in flash. If no configuration is found,
 * an E_NOCONFIG is returned.
 *
 * Returns an error/success code.
 */
error_t load_config() {
	// Look for an entry currently in use.
	int in_use = -1;
	for (int entry = 0; entry < ENTRY_COUNT; entry++) {
		if (config_page.entry_status[entry] == CONFIG_ENTRY_IN_USE) {
			in_use = entry;
			break;
		}
	}

#ifdef TRACE_FLASH
	debug_putchar((unsigned char) in_use);
#endif

	if (in_use == -1) {
		// No entry in use has been found.
		return E_NOCONFIG;
	}

	config = config_page.entries[in_use];

	return E_SUCCESS;
}

/*
 * Saves the configuration to flash.
 */
error_t save_config() {
#ifdef TRACE_FLASH
	debug_string("save");
#endif

	error_t error;

	// Look for the entry last in use and an entry not yet used.
	// Assuming there is only one entry in use.
	int last_in_use = -1;
	int unused = -1;

	for (int entry = 0; entry < ENTRY_COUNT; entry++) {
		if (config_page.entry_status[entry] == CONFIG_ENTRY_IN_USE) {
			last_in_use = entry;
		}

		if (config_page.entry_status[entry] == CONFIG_ENTRY_EMPTY &&
		    unused == -1) {
			unused = entry;
		}
	}

#ifdef TRACE_FLASH
	debug_putchar((unsigned char) last_in_use);
	debug_putchar((unsigned char) unused);
#endif

	flash_unlock();

	// If no entries are free, erase config page and try again
	if (unused == -1) {
#ifdef TRACE_FLASH
		debug_string("erase");
#endif
		error = flash_erase_page(&config_page);
		if (error != E_SUCCESS) goto out;

		return save_config();
	}

	// Save the new configuration.
	_Static_assert(sizeof(config_entry_t) % sizeof(uint16_t) == 0,
		       "config_entry_t must be repadded!");

	error = flash_copy(&config_page.entries[unused], &config,
			   sizeof(config_entry_t) / sizeof(uint16_t));
	if (error != E_SUCCESS) goto out;

#ifdef TRACE_FLASH
	debug_string("copy done");
#endif

	// The configuration was written successfully. Now update the status words.
	error = flash_write_check(config_page.entry_status + last_in_use, CONFIG_ENTRY_OLD);
	if (error != E_SUCCESS) goto out;

	error = flash_write_check(config_page.entry_status + unused, CONFIG_ENTRY_IN_USE);
	if (error != E_SUCCESS) goto out;

#ifdef TRACE_FLASH
	debug_string("status updated");
#endif

out:
	flash_lock();
	return error;
}

/*
 * Strings used in config_valid.
 */
#define CRLF "\r\n"

static const char *ADDRESS_IS_INVALID =
	"The board's address is invalid." CRLF;

static const char *ADDRESS_IS_BROADCAST =
	"Warning: The board's address is the broadcast address." CRLF;

/*
 * Checks if the configuration in config is valid.  Returns 1 if the
 * configuration is valid, 0 otherwise.  This function may print an
 * explanatory message to the debug USART if the configuration is
 * found to be invalid.
 */
int config_valid() {
	int valid = 1;

	// Check if the module has been given a valid address.
	// Warn if the address is the broadcast address.
	if (config.my_address == 0xff) {
		console_write(ADDRESS_IS_INVALID);
		valid = 0;
	}
	if (config.my_address == 0xfe) {
		console_write(ADDRESS_IS_BROADCAST);
	}

	return valid;
}

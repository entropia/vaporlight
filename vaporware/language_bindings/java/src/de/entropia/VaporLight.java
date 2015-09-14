package de.entropia;

import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Objects;

/**
 * Java bindings for the mighty vaporlight.
 * 
 * @author TheDwoon
 *
 */
public final class VaporLight {
	private static final String DEFAULT_HOST = "vaporlight.club.entropia.de";
	private static final int DEFAULT_PORT = 7534;
	private static final byte[] DEFAULT_TOKEN = "sixteen letters.".getBytes(StandardCharsets.US_ASCII);
	private static final int ALPHA_VISIBLE = 255;

	private static final int OP_RGB8 = 0x01;
	private static final int OP_AUTH = 0x02;
	private static final int OP_RGB16 = 0x03;
	private static final int OP_STROBE = 0xFF;

	private final String host;
	private final int port;
	private final byte[] token;

	private Socket socket;
	private DataOutputStream output;

	/**
	 * Connects to the default vaporlight.
	 * 
	 * @throws UnknownHostException if host couldn't be resolved
	 * @throws IOException if an IO error occurs
	 */
	public VaporLight() throws UnknownHostException, IOException {
		this(DEFAULT_HOST, DEFAULT_PORT, DEFAULT_TOKEN);
	}

	/**
	 * Connects to vaporlight.
	 * 
	 * @param host host
	 * @param port port
	 * @param token auth-token
	 * @throws UnknownHostException if host couldn't be resolved
	 * @throws IOException if an IO error occurs
	 */
	public VaporLight(String host, int port, byte[] token) throws UnknownHostException, IOException {
		Objects.requireNonNull(host);
		Objects.requireNonNull(token);

		this.host = host;
		this.port = port;
		this.token = Arrays.copyOf(token, token.length);

		connect();
	}

	/**
	 * Attempts to reconnect 3 times.
	 * 
	 * @throws RuntimeException if reconnect failed
	 */
	private void reconnect() throws RuntimeException {
		close();

		for (int i = 0; i < 3; i++) {
			try {
				connect();
			} catch (IOException e) {
				System.out.println("Failed reconnect attempt: " + (i + 1));
			}

			if (socket != null && socket.isConnected()) {
				return;
			}
		}

		throw new RuntimeException("Cannot reconnect to " + host + ":" + port);
	}

	private void connect() throws UnknownHostException, IOException {
		this.socket = new Socket(host, port);
		this.output = new DataOutputStream(socket.getOutputStream());
	}

	/**
	 * Authenticates you.
	 */
	public void authenticate() {
		if (output == null || socket == null || !socket.isConnected()) {
			throw new IllegalStateException("Not connected");
		}
		
		System.out.println("Try authenticate with " + Arrays.toString(token));

		try {
			output.writeByte(OP_AUTH);
			output.write(token);
			output.flush();
		} catch (IOException e) {
			System.out.println("Could not send command: " + e.getMessage());
			reconnect();
		}
	}

	/**
	 * All changed values will be applied to the lights.
	 */
	public void strobe() {
		if (output == null || socket == null || !socket.isConnected()) {
			throw new IllegalStateException("Not connected");
		}

		try {
			output.writeByte(OP_STROBE);
			output.flush();
		} catch (IOException e) {
			System.out.println("Could not send command: " + e.getMessage());
			reconnect();
		}
	}

	/**
	 * Closes the connection.
	 */
	public void close() {
		try {
			socket.close();
			output.close();
		} catch (IOException e) {
			System.out.println("Connection could not be closed nicely!");
		} finally {
			socket = null;
			output = null;
		}
	}

	/**
	 * Sets the RGB values for a led.
	 * 
	 * @param led led number
	 * @param r red
	 * @param g green
	 * @param b blue
	 */
	public void setRGB8(int led, int r, int g, int b) {
		setRGBA8(led, r, g, b, ALPHA_VISIBLE);
	}

	/**
	 * Sets the RGBA values for a led.
	 * 
	 * @param led led number
	 * @param r red
	 * @param g green
	 * @param b blue 
	 * @param a alpha 
	 */
	public void setRGBA8(int led, int r, int g, int b, int a) {
		if (output == null || socket == null || !socket.isConnected()) {
			throw new IllegalStateException("Not connected");
		}

		try {
			output.writeByte(OP_RGB8);
			output.writeShort(led);
			output.writeByte(r);
			output.writeByte(g);
			output.writeByte(b);
			output.writeByte(a);
		} catch (IOException e) {
			System.out.println("Could not send command: " + e.getMessage());
			reconnect();
			setRGBA8(led, r, g, b, a);
		}
	}

	/**
	 * Sets the RGB values for a led.
	 * 
	 * @param led led number
	 * @param r red
	 * @param g green
	 * @param b blue
	 */
	public void setRGB16(int led, int r, int g, int b) {
		setRGBA16(led, r, g, b, ALPHA_VISIBLE);
	}

	/**
	 * Sets the RGBA values for a led.
	 * 
	 * @param led led number
	 * @param r red
	 * @param g green
	 * @param b blue
	 * @param a alpha
	 */
	public void setRGBA16(int led, int r, int g, int b, int a) {
		if (output == null || socket == null || !socket.isConnected()) {
			throw new IllegalStateException("Not connected");
		}

		try {
			output.writeByte(OP_RGB16);
			output.writeShort(led);
			output.writeShort(r);
			output.writeShort(g);
			output.writeShort(b);
			output.writeShort(a);
		} catch (IOException e) {
			System.out.println("Could not send command: " + e.getMessage());
			reconnect();
			setRGBA16(led, r, g, b, a);
		}
	}
}

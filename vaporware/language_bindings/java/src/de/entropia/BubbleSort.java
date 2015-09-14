package de.entropia;

import java.io.IOException;
import java.net.UnknownHostException;
import java.util.Random;

public final class BubbleSort {
	private static final int LEDS = 35;
	
	public static void main(String[] args) throws InterruptedException, UnknownHostException, IOException {
		Random random = new Random();
		System.out.println("Connecting...");
		VaporLight vaporlight = new VaporLight();
		vaporlight.authenticate();
		System.out.println("Connected");	
		
		while (true) {
			for (int i = 0; i < LEDS / 2.0; i++) {
				vaporlight.setRGB8(i, 0, 0, 0);
				vaporlight.setRGB8(LEDS - i - 1, 0, 0, 0);
				vaporlight.strobe();
				Thread.sleep(250);
			}

			
			// Set leds to random colors.
			LED[] led = new LED[LEDS];
			for (int i = 0; i < LEDS; i++) {
//				led[i] = new LED(random.nextInt(256), random.nextInt(256), random.nextInt(256));
				switch (random.nextInt(3)) {
				case 0:
					led[i] = new LED(255, 0, 0);
					break;
				case 1:
					led[i] = new LED(0, 255, 0);
					break;
				case 2:
					led[i] = new LED(0, 0, 255);
					break;
				}
				vaporlight.setRGB8(i, led[i].r, led[i].g, led[i].b);
				vaporlight.strobe();
				Thread.sleep(200);
			}	
		
			// perform bubble sort
			for (int i = 0; i < LEDS; i++) {
				for (int j = 0; j < LEDS - 1; j++) {
					if (led[j].compareTo(led[j + 1]) > 0) {
						LED l = led[j];
						led[j] = led[j + 1];
						led[j + 1] = l;
						
						vaporlight.setRGB8(j, led[j].r, led[j].g, led[j].b);
						vaporlight.setRGB8(j + 1, led[j + 1].r, led[j + 1].g, led[j + 1].b);
						vaporlight.strobe();

						Thread.sleep(300);
					}
				}
			}
						
			System.out.println("Sorted!");
			Thread.sleep(3000);
		}
	}
	
	private static class LED implements Comparable<LED> {
		private LED(int r, int g, int b) {
			this.r = r;
			this.g = g;
			this.b = b;
		}
		
		private int r;
		private int g;
		private int b;
		
		@Override
		public int compareTo(LED other) {
			return Integer.compare(r + g * 2 + b * 3, other.r + other.g * 2 + other.b * 3);
		}
	}
}

package assign9;

import java.awt.Color;
import java.awt.Font;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

import assign8.ViewInputStream;
import sedgewick.StdDraw;
import studio4.SerialComm;

public class FitBitInfoReceiver {
	final private ViewInputStream vis;

	public FitBitInfoReceiver(InputStream in) {
		vis = new ViewInputStream(in);
	}

	public void run() throws IOException {
		int f = 0;
		int arraycounts = 700;
		double xyzvalues[][] = new double[arraycounts][3];
		int time = 0;
		float temp = 0;
//		int something = 0;
		int steps = 0;
		int timeAsleep = 0;
		int timeOfReset = 0;
		boolean isfull = false;
		boolean pedometer = false;
		DataInputStream in = new DataInputStream(vis);
		while (true) {
			if (in.available() > 0) { // if there is something to be read
				if ((char) vis.read() == 0x21) { // If the magic number '!'
					char header = (char) vis.read();

					switch (header) {

					case 0x30: // DEBUG
						System.out.println();
						int n = in.readShort();
						for (int i = 0; i < n; ++i) {
							System.out.print((char) vis.read());
						}
						break;

					case 0x31: // ERROR STRING
						System.out.println();
						int length = in.readShort();
						for (int i = 0; i < length; ++i) {
							System.out.print((char) vis.read());
						}
						break;

					case 0x32: // TEMP READING
						System.out.println();
						System.out.print("Temperature Reading: ");
						temp = in.readFloat();
						System.out.print(temp);
						break;

					case 0x33: // STEPS COUNT
						System.out.println();
						System.out.print("Steps: ");
						steps = in.readUnsignedShort();
						System.out.print(steps);
						break;

					case 0x34: // TIME SPENT ALSEEP
						System.out.println();
						System.out.print("Time Spent Asleep: ");
						timeAsleep = in.readInt();
						System.out.print(timeAsleep);
						if (timeAsleep == 0) {
							pedometer = true;
						} else {
							pedometer = false;
						}
						break;

					case 0x35: // TOTAL TIME
						System.out.println();
						System.out.print("Total Time: ");
						time = in.readInt();
						System.out.print(time);
						break;

					case 0x36: // acc values
						System.out.println();
						System.out.print("Accelerometer values: ");
						float accelx = in.readFloat();
						float accely = in.readFloat();
						float accelz = in.readFloat();
						// updates the array once array is full, so always
						// keeping most recent 700
						if (isfull) {
							for (int i = 0; i < arraycounts - 1; ++i) {
								xyzvalues[i][0] = xyzvalues[i + 1][0];
								xyzvalues[i][1] = xyzvalues[i + 1][1];
								xyzvalues[i][2] = xyzvalues[i + 1][2];
							}
							xyzvalues[arraycounts - 1][0] = accelx;
							xyzvalues[arraycounts - 1][1] = accely;
							xyzvalues[arraycounts - 1][2] = accelz;
						} else {
							xyzvalues[f % arraycounts][0] = accelx;
							xyzvalues[f % arraycounts][1] = accely;
							xyzvalues[f % arraycounts][2] = accelz;
						}
						f += 1;
						if(f == arraycounts){
							isfull = true;
						}
						System.out.print(accelx);
						System.out.print(", \t");
						System.out.print(accely);
						System.out.print(", \t");
						System.out.print(accelz);
//						System.out.println();
//						something += 1;
//						System.out.print(something);
						break;
						
					case 0x37:	
						timeOfReset = in.readInt();
						break;
					default:
						System.out.println();
						System.out.print("something went wrong");
						System.out.println();
						break;

					}
				}
			}
			drawstuff(xyzvalues, f, isfull, pedometer, steps, time, timeOfReset, temp, timeAsleep);
		}
	}

	public void drawstuff(double[][] xyzvalues, int count, boolean isfull, boolean pedometer, int steps, int time, int timeOfReset, float temp, int timeAsleep) {
		StdDraw.clear();
		StdDraw.enableDoubleBuffering();
		StdDraw.setPenColor(Color.black);
		StdDraw.setPenRadius(0.002);
		StdDraw.setXscale(-100, 720);
		StdDraw.setYscale(-2500, 2500);
		StdDraw.line(0, 0, 720, 0);
		StdDraw.line(0, -4000, 0, 4000);
		// StdDraw.line(1400, -2000, 2050, -2000); ME TRYING TO DO A KEY FOR THE
		// GRAPH
		// StdDraw.line(1400, -2000, 1400, -2500);
		// StdDraw.setPenColor(Color.green);
		// StdDraw.filledSquare(1500, -2100, 20);
		// StdDraw.setPenColor(Color.black);
		Font font = new Font("Arial", Font.BOLD, 12);
		StdDraw.setFont(font);
		
		for (int a = 0; a < 701; a = a + 145) {
			StdDraw.line(a, -50, a, 50);
			StdDraw.text(a, -125, Integer.toString(10*a/145));
		}
		for (int a = -2500; a < 2500; a = a + 250) {
			StdDraw.line(-10, a, 10, a);
			StdDraw.text(-40, a, Double.toString((double)a/1000));
		}
		StdDraw.text(500, -2000, "Steps: " + steps);
		double stepsPerHour = (double)steps*3.6*1_000_000/(time-timeOfReset);
		StdDraw.text(500, -2100, "Steps per hour: " + (int)stepsPerHour);
		StdDraw.text(500, -2200, "Temperature: " + (int)temp + " Celsius");
		StdDraw.text(500, -2300, "Time Asleep: " + timeAsleep/1000 + " seconds");
		
		Font f1 = new Font("Arial", Font.BOLD, 14);
		StdDraw.setFont(f1);

		StdDraw.text(-80, 500, "Acceleration (units of g)", 90);
		StdDraw.text(300, -2400, "Time (seconds)");

		StdDraw.setPenRadius(.005);

		if (isfull) {
			for (int a = 0; a < 700; ++a) {
				StdDraw.setPenColor(Color.blue); // x values
				StdDraw.point(a + 1, xyzvalues[a][0] * 1000);
				StdDraw.setPenColor(Color.red); // y values
				StdDraw.point(a + 1, xyzvalues[a][1] * 1000);
				StdDraw.setPenColor(Color.green); // z values
				StdDraw.point(a + 1, xyzvalues[a][2] * 1000);
			}
		} else {
			for (int a = 0; a < 700; ++a) {
				if (count >= a) {
					StdDraw.setPenColor(Color.blue); // x values
					StdDraw.point(a + 1, xyzvalues[a][0] * 1000);
					StdDraw.setPenColor(Color.red); // y values
					StdDraw.point(a + 1, xyzvalues[a][1] * 1000);
					StdDraw.setPenColor(Color.green); // z values
					StdDraw.point(a + 1, xyzvalues[a][2] * 1000);
				}
			}
		}

		StdDraw.show();

	}

	public static void main(String[] args) {
		// TODO Auto-generated method stub
		try {
			SerialComm s = new SerialComm();
			s.connect("/dev/cu.usbserial-DN01JJ7C"); // Adjust this to be the
														// right port for your
														// machine
			InputStream in = s.getInputStream();
			FitBitInfoReceiver msgr = new FitBitInfoReceiver(in);
			msgr.run();
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

}

using Godot;
using System;
using System.IO.Ports;
using System.Threading;
using System.Numerics;

public partial class MainCode : Sprite3D
{
	static SerialPort _serialPort;
	byte[] writeArr = {(byte)'r', 0x00, 0x00};
	
	// Called when the node enters the scene tree for the first time.
	public override void _Ready()
	{
		_serialPort = new SerialPort("COM5", 9600);
		_serialPort.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);
		_serialPort.Open();
		
		
		if(_serialPort.IsOpen){
			GD.Print("Open");
		}
		
	}

	// Called every frame. 'delta' is the elapsed time since the previous frame.
	public override void _Process(double delta)
	{
		
	}
	
	private static void DataReceivedHandler(
						object sender,
						SerialDataReceivedEventArgs e)
	{
		SerialPort sp = (SerialPort)sender;
		string indata = sp.ReadExisting();
		GD.Print("Data Received:");
		GD.Print(indata);
		
	}
	private void _on_button_button_down()
	{
		_serialPort.Write(writeArr, 0, 3);
		writeArr[2] += 1;
	}

}

// data from https://learn.microsoft.com/en-us/dotnet/api/system.io.ports.serialport?view=dotnet-plat-ext-6.0



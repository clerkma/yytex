% Copyright 2007 TeX Users Group.
% You may freely use, modify and/or distribute this file.

program TERMEMUL;

uses Crt, Dos;

const

	InputBufferSize = 1024; { must be power of two }
	OutputBufferSize = 256; { must be power of two }	
	BaudRate = 1200;
	DataBits = 8;
	Parity = 0;	{ 0 = none, 1= odd, 3 = even }
	StopBits = 1;
	Base = $3F8; { $3F8 = COM1, $2F8 = COM2 }
	IRQ = 4; { 4 = COM1, 3 = COM2 }
	OutputBufferingImplemented = true;

type

	InputBufferType = record
		Chars : array[0..InputBufferSize-1] of char;
		Count, Index : integer;
		Overflow : boolean;
	end;

	OutputBufferType = record
		Chars : array[0..OutputBufferSize-1] of char;
		Count, Index : integer;
		Overflow : boolean;
	end;

var

	InputBuffer : InputBufferType;
	OutputBuffer : OutputBufferType;
	SaveInterruptVector : pointer;
	InterruptVectorNumber : integer;
	InChar : char;
	OutChar : integer;
	LogFile : text;
	Logging : boolean;

procedure InterruptHandler;
interrupt;
var
	InterruptIdentification : Byte;
	LineStatus : Byte;
	Ch : Byte;
	LoopCount : integer;
begin
	LoopCount := 0;
	repeat
		InterruptIdentification := Port[Base+2] and 7;
		LineStatus := Port[Base+5];
		if (InterruptIdentification = 4) or ((LineStatus and 1) <> 0) then
		with InputBuffer do
		begin
			Ch := Port[Base];
			if Count < InputBufferSize then
			begin
				Chars[Index] := Chr(Ch);
				Index := (Index+1) and (inputBufferSize-1);
				Count := Count+1;
			end
			else
				Overflow := true;
		end;
		if (interruptIdentification = 2) or ((LineStatus and $20) <> 0) then
		with OutputBuffer do
		begin
			if Count > 0 then
			begin
				Port[Base] := Ord(Chars[Index]);
				Index := (Index+1) and (OutputBufferSize-1);
				Count := Count-1;
			end
			else Count := -1;
		end;
		LoopCount :=  LoopCount + 1l
	until ((InterruptIdentification and 1) = 1) or (LoopCount = 3);
	if IRQ<8 then Port[$20] := $20; { reset PIC }
end;

function Key : integer;
var
	k : char;
begin
	Key := 0;
	if KeyPressed then
	begin
		k := ReadKey;
		if k = #0 then
			Key := 256 + ord(ReadKey)
		else
			Key := ord(k);
	end;
end;

procedure DisableIntrrupts; inline($FA);
procedure EnableIntrrupts; inline($FB);

begin

	if ParamCount >0 then
	begin
		Assign(LogFile, ParamStr(1));
		Rewrite(LogFile);
		Logging := true;
	end
	else Logging := false;
	InputBuffer.Count := 0;
	InputBuffer.Index := 0;
	InputBuffer.Overflow := false;
	OutputBuffer.Count := 0;
	OutputBuffer.Index := 0;
	OutputBuffer.Overflow := false;
	if IRQ<8 then InterruptVectorNumber := IRQ+8
	else InterruptVectorNumber := IRQ+12;
	GetIntVec(InterruptVectorNumber, SavedInterruptVector);
	SetIntVec(InterruptVectorNumber, @InterruptHandler);
	DisableInterrupts;
	Port[Base+3] := $80; { select baud rate count registers }
	Port[Base] := (115200 div BaudRate) and $FF;
	Port[Base+1] := (115200 div BaudRate) div 256;
	Port[Base+3] := Parity*8 + (stopBites-1)*4 + (DataBits-5);
	Port[Base+4] := 8+3;	{ enable interrupt conditions }
	Port[Base+1] := 3; { define interrupt conditions }
	OutChar := Port[Base]; { clean out Received DataRegister }
	if IRQ<8 then { enable interrupt via PIC }
		Port[$21] := Port[$21] and ($FF - (1 shl IRQ))
	else
		Port[$A1] := Port[$A1] and ($FF - (1 shl (IRQ-8)));
	EnableInterrupts;
	OutChar := 0;

	repeat

		if OutChar = 0 then OutChar := Key;

		if (0<OutChar) and (OutChar<127) then
		begin
			DisableInterrupts;
			if Count = OutBufferSize then Overflow := true
			else
			begin
				if Count = -1 thenPort[Base] := OutChar
				else Chars[(Index+Count) and (OutBufferSize-1)] := Chr(OutChar);
				Count := Count+1;
			end;
			EnableInterrupts;
			OutChar := 0;
		end
		else if (Port[Base+5] and $20) <> 0 then
		begin
			Port[Base] := OutChar;
			OutChar := 0;
		end;
	end;
	with InputBuffer do if Count > 0 then
	begin
		DisableInterrupts;
		InChar := Chars[(Index-Count) and (InputBufferSize-1);
		Count := Count-1;
		EnableInterrupts;
		if InChar <> #0 then
		begin
			Write(InChar);
			if Logging then Write(LogFile, InChar);
		end;
	end;

until OutChar = 301 { Alt-X };

Port[Base+4] := 0; {disable interrupts, clear RTS and DTR }
SetIntVec(InterruptVectorNumbr, SavedInterruptVector);
	{ restore interrupt vector }
	if IRQ<8 then 	{ restore PIC }
		Port[$21] := Port[$21] or (1 shl IRQ)
	else
		Port[$A1] := Port[$A1] or (1 shl (IRQ-8));
	if Logging then Close(LogFile);
end.

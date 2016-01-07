//LCD Connections
#define LCD_RS  PORTA.B2   // RS
#define LCD_EN  PORTD.B6   //Enable
#define LCD_D0  PORTC.B0   //Data Bit 0
#define LCD_D1  PORTC.B1   //Data Bit 1
#define LCD_D2  PORTC.B2   //Data Bit 2
#define LCD_D3  PORTC.B3   //Data Bit 3
#define LCD_D4  PORTC.B4   //Data Bit 4
#define LCD_D5  PORTC.B5   //Data Bit 5
#define LCD_D6  PORTC.B6   //Data Bit 6
#define LCD_D7  PORTC.B7   //Data Bit 7

char disp2_12='0';
unsigned short int count=0,ifFaculty=0,flag=0,RollNumber=0;
//char message[]="Show Card";
char RFID_No[13];
char rfid_T[]="120088F349";
char T_id[]="EMP001";
char rfid_S1[]="290091318C";
char rfid_S2[]="290091114B";
char rfid_S3[]="2900911BE7";
char rfid_S4[]="2900913990";
char date[9],C_No[2],Sem_No[2];
char  keypadPort at PORTB;
char  keypadPort_Direction at DDRB;
//unsigned short kp;

void EEPROM1_write(unsigned int uiAddress,unsigned char Rno)
{
	EEARH=0x00;
	EEARL=uiAddress;
	EEDR=Rno;
	EECR.B2=1;
	EECR.B1=1;
	while(EECR&0x02);
	delay_ms(100);
}

void LCD_data(unsigned char Data)
{
/*
	PORTC=Data&0xF0; // Send Higher nibble (D7-D4)

	LCD_RS=1;       // Register Select =1 (for data select register)
	LCD_EN=1;      //Enable=1 for H to L pulse
	delay_us(5);
	LCD_EN=0;

	PORTC=((Data<<4)&0xF0); // Send Lower nibble (D3-D0)

	LCD_EN=1;               //Enable=1 for H to L pulse
	delay_us(5);
	LCD_EN=0;

	delay_us(100);
*/

	PORTC=Data; 

	LCD_EN=1;
	delay_us(5);
	LCD_EN=0;

	delay_us(100);
}


//LCD Print
void LCD_Print(char * str)
{
	unsigned char i=0;

	// Till NULL charecter is reached, take each character
	while(*(str+i)!=0)
	{
		LCD_data(*(str+i)); // Data sent to LCD data register
		i++;
		delay_ms(50);    //checked
	}
}

//LCD Command
void lcdcommand(unsigned char command)
{
/*
	PORTC=command&0xF0; // Send Higher nibble (D7-D4)
	LCD_RS=0; // Register Select =0 (for Command register)
	LCD_EN=1; //Enable=1 for H to L pulse
	delay_us(5);
	LCD_EN=0;
	delay_us(100);

	PORTC=((command<<4)&0xF0);  // Send Lower nibble (D3-D0)
	LCD_EN=1; //Enable=1 for H to L pulse
	delay_us(5);
	LCD_EN=0;

	delay_us(40);
*/

	PORTC=command; 
	LCD_RS=0; // Register Select =0 (for Command register)
	LCD_EN=1; //Enable=1 for H to L pulse
	delay_us(5);
	LCD_EN=0;
	delay_us(100);

}


// Cursor Posotion
void Cursor_Position(unsigned short int x,unsigned short int y)
{
	unsigned char firstcharadd[] ={0x80,0xC0,0x90,0xD0};        // First line address 0X80, Second line address 0XC0, Third line address 0X90 & 4th line add D0
	lcdcommand((firstcharadd[x-1]+y-1));
}

void clear()
{
	lcdcommand(0x01);
	delay_ms(2);
}

//LCD Iniatialize
void LCD_Initialize()
{
	LCD_EN=0;

/*
	lcdCommand(0x33); // Initialize LCD for 4 bit mode
	lcdCommand(0x32); // Initialize LCD for 4 bit mode
	lcdCommand(0x28); // Initialize LCD for 5X7 matrix mode
*/
	lcdCommand(0x38); //Function Set: 8-bit, 2 Line, 5x7 Dots	0x38	56
	lcdCommand(0x0E); //Display on,cursor blinking
	clear();
	lcdCommand(0x06); //Shift cursor to right
	//lcdCommand(0x80);
	//Cursor_position(1,3);
}



unsigned char EEPROM1_read(unsigned int uiAddress)
{
	EEARH=0x00;
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE));
	/* Set up address register */
	EEARL = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from data register */
	return EEDR;
}



void usart_read_initialize()
{
	UCSRB=0x10; // Rx Enable
	UCSRC=0x86; // Data Size : 8-bit, Stop Bit:1,No parity
	UBRRL=0x33; //    X= (Fosc/(16(Desired Baud Rate)))-1
				//     = (8*10^6/(16 *9600))-1
				//     = 52.08-1
				//     = 51 (Dec)
	//Here, URSEl=0, so Fosc is divided by 16 if it was 1 Fosc would
	//Have been diveded by 8
}

void usart_write_initialize()
{
	UCSRB=0x08; //Tx Enable
	UCSRC=0x86; // Data Size : 8-bit, Stop Bit:1,No parity
	UBRRL=0x33; //  X  = (Fosc/(16(Desired Baud Rate)))-1
			    //     = (8*10^6/(16 *9600))-1
			    //     = 52.08-1
			    //     = 51 (Dec)
	//Here, URSEl=0, so Fosc is divided by 16 if it was 1                     //Fosc would Have been divided by 8
}


void usart_send(unsigned char ch)
{
	while(UCSRA.B5==0); // Wait till UDR is empty
	UDR=ch; //Write the value to be Tx
}




int checkIfFaculty()
{
	unsigned short int i = 0;
	for(i=0;i<10;i++)
	{
		if(RFID_No[i]!=rfid_T[i])
			return 0;
	}
	return 1;
}

int RFID_RollNumber_Mapping()
{
	unsigned short int i = 0,flag=0;

	for(i=0;i<10;i++)
	{
		if(RFID_No[i]!=rfid_S1[i])
		{
			i=0;
			flag=1;
			break;
		}
	}
	
	if(flag!=1)
		return 1;
	else
		flag=0;
	
	for(;i<10;i++)
	{
		if(RFID_No[i]!=rfid_S2[i])
		{
			i=0;
			flag=1;
			break;
		}
	}
	
	if(flag!=1)
		return 2;
	else
		flag=0;
		
	for(;i<10;i++)
	{
		if(RFID_No[i]!=rfid_S3[i])
		{
		   i=0;
		   flag=1;
		   break;
		}
	}
	
	if(flag!=1)
		return 3;
	else
		flag=0;

	for(;i<10;i++)
	{
		if(RFID_No[i]!=rfid_S4[i])
		{
			i=0;
			flag=1;
			return 0;
		}
	}
	
	if(flag!=1)
		return 4;
	else
		return 0;
}

int checkValidInput()
{
	unsigned short int i = 0;
	for(; i<12; i++)
		RFID_No[i] = RFID_No[i]>64 ? RFID_No[i]-55 : RFID_No[i]-48;


	if( ( ( (RFID_No[0] ^ RFID_No[2]) ^ RFID_No[4] ) ^ RFID_No[6] ) ^ RFID_No[8] == RFID_No[10]&& ( ( (RFID_No[1] ^ RFID_No[3]) ^ RFID_No[5] ) ^ RFID_No[7] ) ^ RFID_No[9] == RFID_No[11])
	{
		for(i = 0; i<12; i++)
			RFID_No[i] = RFID_No[i]>9 ? RFID_No[i]+55 : RFID_No[i]+48;
		
		return 1;
	}

	  return 0;
}
int CheckAddress()
{
	char mem;
	mem=EEPROM1_read(24);
	
	if(mem!='E')
		return 0;
	else
	{
		mem=EEPROM1_read(54);
		if(mem!='E')
			return 30;
		else
		{
			mem=EEPROM1_read(84);
			if(mem!='E')
				return 60;
			else
			{
				mem=EEPROM1_read(114);
				if(mem!='E')
					return 90;
				else
				{
					mem=EEPROM1_read(144);
					if(mem!='E')
						return 120;
				}
			}
		}
	}
}

char Press_Value(char num)
{
	switch (num)
	{
		//case 10: kp = 42; break;  // '*'   // Uncomment this block for keypad4x3
		//case 11: kp = 48; break;  // '0'
		//case 12: kp = 35; break;  // '#'
		//default: kp += 48;

		case  1: num = 49; return num; // 1        // Uncomment this block for keypad4x4
		case  2: num = 50; return num; // 2
		case  3: num = 51; return num; // 3
		case  4: num = 65; return num; // A
		case  5: num = 52; return num; // 4
		case  6: num = 53; return num; // 5
		case  7: num = 54; return num; // 6
		case  8: num = 66; return num; // B
		case  9: num = 55; return num; // 7
		case 10: num = 56; return num; // 8
		case 11: num = 57; return num; // 9
		case 12: num = 67; return num; // C
		case 13: num = 42; return num; // *
		case 14: num = 48; return num; // 0
		case 15: num = 35; return num; // #
		case 16: num = 68; return num; // D

    }

}


void main()
{ 
	//DDRD.B0=;
	//DDRD.B1=1;
	//Set-up PORTS for LCD
	DDRC=0xFF;  // For D3-D0
	DDRA.B2=1;  //For RS
	DDRD.B6=1;  //For Enable
	
	char kp;
	unsigned short int i,memoryCount=0,mem_offset,loop_help;
	RFID_No[12] = '\0';
	date[2]=date[5]='/';
	date[8]=C_No[1]=Sem_No[1]='\0';

	usart_read_initialize();
	LCD_Initialize(); //Initialize
	Cursor_Position(2,1);
	LCD_Print("Booting up...");
	Delay_ms(2000);
	clear();
	Cursor_Position(2,1);
	LCD_Print("Welcome");
	Delay_ms(1000);
	clear();
	mem_offset=CheckAddress();
	Cursor_Position(3,1);
	LCD_Print("Enter Date:-");
	Cursor_Position(4,1);
	LCD_Print("_ _/_ _/_ _");
	Delay_ms(2000);

	Cursor_Position(4,1);
	do{
		kp = Keypad_Key_Click();
	}while(!kp);
	date[0]=Press_Value(kp);

	Cursor_Position(4,3);
	do{
		kp = Keypad_Key_Click();
	}while(!kp);
	date[1]=Press_Value(kp);

	Cursor_Position(4,5);
	do
	kp = Keypad_Key_Click();
	while(!kp);
	date[3]=Press_Value(kp);
	
	Cursor_Position(4,7);
	do{
		kp = Keypad_Key_Click();
	}while(!kp);
	date[4]=Press_Value(kp);
	
	Cursor_Position(4,9);
	do{
		kp = Keypad_Key_Click();
	}while(!kp);
	date[6]=Press_Value(kp);
	
	Cursor_Position(4,11);
	do{
		kp = Keypad_Key_Click();
	}while(!kp);
	date[7]=Press_Value(kp);

	clear();
	LCD_Print(date);
	Delay_ms(2000);
	memoryCount=mem_offset;
	loop_help=memoryCount+9;
	
	for(;memoryCount<loop_help;memoryCount++)
		EEPROM1_write(memoryCount,date[memoryCount]);
	clear();
	loop_help=memoryCount+9;
	for(memoryCount=0;memoryCount<loop_help;memoryCount++)
		date[memoryCount]=EEPROM1_read(memoryCount);
	LCD_Print(date);
	clear();
	Delay_ms(2000);
	LCD_Print("Date Entered");
	Delay_ms(500);

	LCD_Print("Enter Class No.");
	
	do{
		kp = Keypad_Key_Click();
	}while(!kp);
	
	C_No[0]=Press_Value(kp);
	loop_help=memoryCount+2;
	
	for(;memoryCount<loop_help;memoryCount++)
		EEPROM1_write(memoryCount,C_No[memoryCount-loop_help]);
		
	do{
		Cursor_Position(1,1);
		LCD_Print("Enter Faculty ID");
		Cursor_Position(4,1);
		LCD_Print("options");
		Delay_ms(500);
		for(i=0;i<12;i++)
		{
			while(UCSRA.B7==0); // Wait till Data is received
			RFID_No[i]=UDR;
		}
		clear();
		if(checkValidInput() == 0)
		{
			Cursor_position(1,1);
			LCD_Print("Cannot detect");

			Cursor_position(2,1);
			LCD_Print("Show again...");

			Delay_ms(3000);
			clear();
			continue;
		}
		ifFaculty=checkIfFaculty();
		if(ifFaculty==1)
		{
			Cursor_Position(1,1);
			LCD_Print("Welcome");

			Cursor_Position(2,1);
					 LCD_Print("Prof. Nishit");

			Cursor_Position(3,5);
			LCD_Print("Pandya");
			Delay_ms(2000);
			Cursor_position(1,1);
			clear();
		}
		else
		{
			clear();
			Delay_ms(500);
			continue;
		}
	}while(ifFaculty==0);

	while(1)
	{
		clear();
		Cursor_Position(1,1);
		LCD_Print("Student :-");
		Cursor_Position(2,1);
		LCD_Print("Show Card");
		Delay_ms(50);

		for(i=0;i<12;i++)
		{
			while(UCSRA.B7==0); // Wait till Data is received
			RFID_No[i]=UDR;
		}
		clear();

		if(checkValidInput() == 0)
		{
			Cursor_position(1,1);
			LCD_Print("Cannot detect");

			Cursor_position(2,1);
			LCD_Print("Show again...");

			Delay_ms(3000);
			continue;
		}
		RollNumber=RFID_RollNumber_Mapping();
		
		if(RollNumber==0)
		{
			LCD_Print("Invalid Student");
		}
		else if (RollNumber==1)
		{
			EEPROM1_write(memoryCount++,'1');
			clear();
			Cursor_Position(2,1);
			LCD_Print("Roll Number 1");
			Cursor_Position(3,1);
			LCD_Print("Present");
			Delay_ms(2000);
			clear();
		}
		else if (RollNumber==2)
		{
			EEPROM1_write(memoryCount++,'1');
			clear();
			Cursor_Position(2,1);
			LCD_Print("Roll Number 2");
			Cursor_Position(3,1);
			LCD_Print("Present");
			Delay_ms(2000);
			clear();
		}
		else if (RollNumber==3)
		{
			EEPROM1_write(memoryCount++,'1');
			clear();
			Cursor_Position(2,1);
			LCD_Print("Roll Number 3");
			Cursor_Position(3,1);
			LCD_Print("Present");
			Delay_ms(2000);
			clear();
		}
		else if (RollNumber==4)
		{
			EEPROM1_write(memoryCount++,'1');
			clear();
			Cursor_Position(2,1);
			LCD_Print("Roll Number 4");
			Cursor_Position(3,1);
			LCD_Print("Present");
			Delay_ms(2000);
			clear();
		}
		ifFaculty=checkIfFaculty();
		if(ifFaculty==1)
		{
			Cursor_position(1,1);
			LCD_Print("Attendance done");
			Cursor_Position(2,1);
			LCD_Print("Thank you!");
			Delay_ms(2000);
			clear();
			Cursor_Position(3,1);
			//LCD_Print("Thank You!");
			Delay_ms(3000);
			break;
		}
	}
	
	LCD_Print("Enter Sem No.");
	do
	kp = Keypad_Key_Click();
	while(!kp);
	S_No[0]=Press_Value(kp);

	memoryCount+=5;
	loop_help=memoryCount+4;
	for(;memoryCount<loop_help;memoryCount++)
	EEPROM1_write(memoryCount,C_No[memoryCount-loop_help]);
	usart_write_initialize();

	for(i=1;i<=4;i++)
	{
		RollNumber=EEPROM1_read(i);
		usart_send(RollNumber);  //Send data

		if (i>=4)  // Till string Length
		{
			i=0;
			delay_ms(2000);
			LCD_Print("Data Read");
			Delay_ms(2000);
			break;
		}
	}

}
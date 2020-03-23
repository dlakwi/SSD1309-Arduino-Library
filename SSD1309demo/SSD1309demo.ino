// SSD1309demo.ino
// 
// Real OLED Display, 2.42" 128*64 12864 Graphic LCD Module Screen LCM Screen SSD1309 Controller Support Parallel, SPI, I2C / IIC
// https://www.aliexpress.com/item/32919030007.html
// 2020-03-01
// 
// ----------------
// Based on:
// Code written for Seeeduino v4.2 runnung at 3.3v
// https://www.seeedstudio.com/Seeeduino-V4-2-p-2517.html
// 2020-03-01
// 
// CRYSTALFONTZ CFAL12864K 128x64 OLED
// 
// ref: https://www.crystalfontz.com/product/cfal12864k
// 
// 2017-03-25 Brent A. Crosby
// 2019-10-25 Kelsey Zaches and Trevin Jorgenson
// 2020-03-01 Donald Johnson
// ----------------

#include <Arduino.h>
#include <avr/pgmspace.h>

// ----------------

// Wiring
//
// Arduino UNO pin    Surenoo module           SSD1309 OLED pin 
//                                              1   NC          
//   GND               1   VSS                  2   VSS = GND   
//                                              3   NC          
//                                              4   NC          
//                                              5   NC          
//                                              6   NC          
//                                              7   NC          
//                                              8   NC          
//                                              9   NC          
//                                             10   NC          
//   3V3               2   VDD                 11   VDD = 3V3   
//                    --                       12   BS1         
//                    --                       13   BS2         
//                                             14   NC          
//    A4              15   CS  Chip Select     15   CS          
//    A3              16   RES Reset           16   RES         
//    A2               4   DC  Data/Command    17   DC          
//    A0               5   WR  Write           18   RW          
//    A1               6   RD  Read            19   RD          
//     8               7   D0                  20   D0          
//     9               8   D1                  21   D1          
//     2               9   D2                  22   D2          
//     3              10   D3                  23   D3          
//     4              11   D4                  24   D4          
//     5              12   D5                  25   D5          
//     6              13   D6                  26   D6          
//     7              14   D7                  27   D7          
//                    --                       28   IREF        
//                    --                       29   VCOMH       
//                    --                       30   VCC = 13V~  
//                                             31   NC          
//                                                              
//                     3   NC                                   
//                    17   NC                                   
//                    18   NC                                   
//                    19   NC                                   
//                    20   FG  Frame Ground                     
//

// ----------------

// Scroll instructions
// todo ..
// 
// setup continuous horizontal scroll
// 0x26  scroll right
// 0x27  scroll left
//   0x00
//   0b00000xxx  start page
//   0b00000xxx  time interval = { 111, 100, 101, 110, 000, 001, 010, 011 } ==> { 1, 2, 3, 4, 5, 64, 128, 256 } frames
//   0b00000xxx  end page
//   0x00
//   0bxxxxxxxx  start column
//   0bxxxxxxxx  end column
// 
// setup continuous vertical and horizontal scroll
// 0x29  scroll vertical and right
// 0x2A  scroll vertical and left
//   0x0000000x  horizontal scroll offset
//   0b00000xxx  start page
//   0b00000xxx  time interval = { 111, 100, 101, 110, 000, 001, 010, 011 } ==> { 1, 2, 3, 4, 5, 64, 128, 256 } frames
//   0b00000xxx  end page
//   0b000xxxxx  vertical scroll offset
//   0bxxxxxxxx  start column
//   0bxxxxxxxx  end column
// 
// deactivate scroll
// 0x2E
// 
// activate scroll
// 0x2F
// 
// set vertical scroll area
// 0xA3
//   0b00xxxxxx  fixed top rows
//   0b0xxxxxxx  scrolling rows
// 
// horizontal scroll by one column
// 0x2C  scroll right
// 0x2D  scroll left
//   0x00
//   0b00000xxx  start page
//   0b00000xxx  time interval = { 111, 100, 101, 110, 000, 001, 010, 011 } ==> { 1, 2, 3, 4, 5, 64, 128, 256 } frames
//   0b00000xxx  end page
//   0x00
//   0bxxxxxxxx  start column
//   0bxxxxxxxx  end column
// 

// ----------------

// DDRx, PORTx, PINx i/o was tried, but OLED operation was erratic
// it's possible uC i/o is too fast without delays between instructions
// so, Arduino pinMode(), digitalRead(), and digitalWrite() are used

// OLED control pins - these are active LO, so "0" is is on/active state
#define WR_on        digitalWrite( A0, 0 )
#define WR_off       digitalWrite( A0, 1 )
#define RD_on        digitalWrite( A1, 0 )
#define RD_off       digitalWrite( A1, 1 )
#define DC_command   digitalWrite( A2, 0 )
#define DC_data      digitalWrite( A2, 1 )
#define RST_on       digitalWrite( A3, 0 )
#define RST_off      digitalWrite( A3, 1 )
#define CS_select    digitalWrite( A4, 0 )
#define CS_deselect  digitalWrite( A4, 1 )

// OLED data pins 0~7 ==> Arduino UNO pins 8~9,2~7
byte pin[] = { 8, 9, 2, 3, 4, 5, 6, 7 };  // do not use 0 and 1 (Rx and Tx)

// set the Arduino data pins to inputs
void setAsInput( void )
{
  for ( int i = 0; i < 8; i++ )
    pinMode( pin[i], INPUT );
}

// set the Arduino data pins to outputs
void setAsOutput( void )
{
  for ( int i = 0; i < 8; i++ )
    pinMode( pin[i], OUTPUT );
}

// initialize the Arduino control and data pins
void initIO( void )
{
  // all command pins are set to be outputs
  pinMode( A0, OUTPUT );
  pinMode( A1, OUTPUT );
  pinMode( A2, OUTPUT );
  pinMode( A3, OUTPUT );
  pinMode( A4, OUTPUT );
// all data pins are initially set to be outputs
  setAsOutput();
  setData( 0x00 );

  // Set the ports to reasonable starting states
  CS_select;
  RST_off;
  DC_command;
  WR_off;
  RD_off;
  CS_deselect;
}

// set the Arduino data pins to byte d
void setData( byte d )
{
  for ( int i = 0; i < 8; i++ )
    digitalWrite( pin[i], ( ( d >> i ) & 0x01 ) );
}

// get a data byte from the Arduino data pins
byte getData( void )
{
  byte d = 0x00;
  for ( int i = 0; i < 8; i++ )
    d |= ( digitalRead( pin[i] ) << i );
  return d;
}

#define del delayMicroseconds(5)

// write the command byte c to the OLED display
void writeCommand( byte c )
{
  RD_off;                // not reading
  CS_select;             // enable OLED control
  DC_command;            // command

  setData( c );          // present command
  del;
  WR_on;
  del;
  WR_off;                // OLED command is written as WR goes off

  CS_deselect;           // disable OLED control
}

// write the data byte d to the OLED display
void writeData( byte d )
{
  RD_off;                // not reading
  CS_select;             // enable OLED control
  DC_data;               // data

  setData( d );          // present data
  del;
  WR_on;
  del;
  WR_off;                // OLED data is written as WR goes off

  CS_deselect;           // disable OLED control
}

// write N bytes to the OLED display
void writeData( byte d, byte N )
{
  RD_off;                // not reading
  CS_select;             // enable OLED control
  DC_data;               // data

  setData( d );          // present data
  for ( byte i = 0; i < N; i++ )
  {
    del;
    WR_on;
    del;
    WR_off;              // OLED data is written as WR goes off
  }

  CS_deselect;           // disable OLED control
}

// do a dummy read only after setting the column address
boolean need_dummy_read = false;

// read a data byte from the OLED display
byte readData( void )
{
  byte response = 0x00;

  setData( 0x00 );
  setAsInput();          // set the Arduino data pins to inputs

  WR_off;                // not writing
  CS_select;             // enable OLED control
  DC_data;               // data

  if ( need_dummy_read )
  {
    RD_on;
    del;
    RD_off;
    del;
    need_dummy_read = false;
  }
  RD_on;
  del;
  RD_off;                // OLED data is presented as RD goes off
  del;
  response = getData();  // read the OLED into the Arduino data pins

  CS_deselect;           // disable OLED control

  setAsOutput();         // set the Arduino data pins back to outputs

  return response;
}

// ----------------

// fill the OLED display RAM

void fillRAM( byte b )
{
  byte i, j;

  for ( i = 0; i < 8; i++ )
  {
    setPage( i );
    setColumn( 0 );
    writeData( b, 128 );
  }
}

// ----------------

#define ADDR_MODE 2  // 0:horizontal, 1:vertical, 2:page

void initDisplay( void )
{
  // reset the display
  RST_off;
  _delay_ms( 10 );
  RST_on;
  _delay_ms( 100 );
  RST_off;
  _delay_ms( 100 );

  writeCommand( 0xAE );  // display off
  writeCommand( 0xD5 );
  writeCommand( 0xA0 );  // clock divide ratio (0x00=1) and oscillator frequency (0x8)
  writeCommand( 0x40 );  // set display start line to 0
  writeCommand( 0x20 );
  writeCommand( ADDR_MODE );  // page addressing mode

  writeCommand( 0xA1 );  // segment remap a0/a
  writeCommand( 0xC8 );  // C0: scan dir normal, C8: reverse

  writeCommand( 0xDA );
  writeCommand( 0x12 );  // com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5)

  writeCommand( 0x81 );
  writeCommand( 0x6F );  // [2] set contrast control
  writeCommand( 0xD9 );
  writeCommand( 0xD3 );  // [2] pre-charge period 0x022/f
  writeCommand( 0xDB );
  writeCommand( 0x20 );  // vcomh deselect level
                         // if vcomh is 0, then this will give the biggest range for contrast control - issue #98
                         // restored the old values for the noname constructor, because vcomh=0 will not work for all OLEDs - issue #116

  writeCommand( 0x2E );  // Deactivate scroll
  writeCommand( 0xA4 );  // output ram to display
  writeCommand( 0xA6 );  // none inverted normal display mode

  fillRAM( 0x00 );       // clear display before turning it on

  writeCommand( 0xAF );  // display on
}

// ----------------

#if (ADDR_MODE==2)

void setAddr( byte page, byte lCol, byte hCol )
{
  writeCommand( page );  // Set Page Start Address
  writeCommand( lCol );  // Set Lower Column Start Address
  writeCommand( hCol );  // Set Higher Column Start Address
}

#else

void setPageAddr( byte startAddr, byte endAddr )
{
  writeCommand( startAddr );
  writeCommand( endAddr );
}

void setColAddr( byte startAddr, byte endAddr )
{
  writeCommand( startAddr );
  writeCommand( endAddr );
}

#endif

void setHome( void )
{
#if (ADDR_MODE==2)
  setAddr( 0xB0, 0, 0x10 );
#else
  writeCommand( 0x22 );  // set Page Address
  setPageAddr( 0, 7 );
  writeCommand( 0x21 );  // set Column Address
  setColAddr( 0, 127 );
#endif
}

// Instruction Setting

void setColumn( byte c )
{
  writeCommand( 0x00 + c % 16 );  // set Lower  Column Start Address for Page Addressing Mode  Default => 0x00
  writeCommand( 0x10 + c / 16 );  // set Higher Column Start Address for Page Addressing Mode  Default => 0x10
  need_dummy_read = true;         // for readData()
}

void setPage( byte p )
{
  writeCommand( 0xB0 | p );  // set Page Start Address for Page Addressing Mode  Default => 0xB0 (0x00)
}

void setColumns( byte c0, byte c1 )
{
  writeCommand( 0x21 );  // set Column Address
  writeCommand( c0 );    //   Default => 0x00 (Column Start Address)
  writeCommand( c1 );    //   Default => 0x7F (Column End Address)
}

void setPages( byte p1, byte p2 )
{
  writeCommand( 0x22 );  // set Page Address
  writeCommand( p1 );    //   Default => 0x00 (Page Start Address)
  writeCommand( p2 );    //   Default => 0x07 (Page End Address)
}

// ----------------

// set the column and page for the pixel coordinate (x,y)

void setXY( byte x, byte y )
{
  y = y / 8;
  setPage( y );
  setColumn( x );
}

static boolean checkXY = false;

// set the pixel at (x,y) to colour
// read-modify-write method

void setPixel( byte x, byte y, byte colour )
{
  if ( checkXY && ( x > 127 ) && ( y > 63 ) )
    return;

  byte temp;
  setXY( x, y );
  temp = readData();  // the pixel is one of 8 bits - we want to change only this pixel
  setXY( x, y );
  if ( colour )
  {
    temp = temp |  (1 << (y % 8));  // 1 ~ set
  }
  else
  {
    temp = temp & ~(1 << (y % 8));  // 0 ~ clear
  }
  writeData( temp );
}

byte getPixel( byte x, byte y )
{
  if ( checkXY && ( x > 127 ) && ( y > 63 ) )
    return 0x00;

  setXY( x, y );
  byte b = readData();
  return ( b & ( 1 << (y%8) ) ) ? 1 : 0;
}

// ----------------

// drawChar
// 5x7 character in a 6x8 box
// any column (x = 0~127)
// row = 0~7 (y = 0/8/16/24/..)

#include "font.h"

void drawChar( byte x, byte y, byte ch )
{
  y = y / 8;

  if ( ( ch >= 0x20 ) && ( ch <= 0x7E ) )  // normal ASCII Char
  {
    ch -= 0x20;                            // subtract to match index
  }
  else if ( ch >= 0xA0 )                   // extended UTF8 char
  {
    ch -= 0x41;                            // subtract to match index
  }
  else                                     // char not available
  {
    ch = '?' - 0x20;                       // make it a ?
  }

  setPage( y );
  setColumn( x );

  for ( byte i = 0; i < 5; i++ )
  {
    writeData( pgm_read_byte( &font[ch][i] ) );
  }
  writeData( 0 );                                // blank column
}

// drawGraphic
// 6x8 graphic in a 6x8 box
// any column (x = 0~127)
// row = 0~7 (y = 0/8/16/24/..)

#include "font2.h"

void drawGraphic( byte x, byte y, byte gr )
{
  y = y / 8;

  if ( ( gr >= 0x00 ) && ( gr <= 0x1F ) )       // low graphic
  {
    gr -= 0;                                    // subtract to match index
  }
  else if ( ( gr >= 0xB0 ) && ( gr <= 0xFF ) )  // High graphic
  {
    gr -= 0x90;                                 // subtract to match index
  }
  else                                          // char not available
  {
    gr = 0x00;                                  // make it a blank (00)
  }

  setPage( y );
  setColumn( x );

  for ( byte i = 0; i < 6; i++ )
  {
    writeData( pgm_read_byte( &graphic[gr][i] ) );
  }
}

// draw a string

void drawString( byte x, byte y, char* aString )
{
  char* Src_Pointer = aString;    // copy the pointer, because we will increment it

  while ( *Src_Pointer != '\0' )  // until string end
  {
    drawChar( x, y, (byte)*Src_Pointer );
    Src_Pointer++;
    x += 6;
  }
}

// draw a string residing in PROGMEM

void drawString_P( byte x, byte y, const char* aString )
{
  byte tmp;
  const char* Src_Pointer = aString;  // copy the pointer, because we will increment it

  tmp = pgm_read_byte( Src_Pointer );
  while ( tmp != '\0' )
  {
    drawChar( x, y, tmp );
    Src_Pointer++;
    tmp = pgm_read_byte( Src_Pointer );
    x += 6;
  }
}

#ifdef _USE_PRINTF

void printf( byte x, byte y, const char* __fmt, ... )
{
  char aString[64];
  va_list argumentlist;
  va_start( argumentlist, __fmt );
  sprintf( aString, __fmt, argumentlist );
  va_end( argumentlist );
  drawString( x, y, aString );
}

void printf_P( byte x, byte y, const char* __fmt, ... )
{
  char aString[64];
  va_list argumentlist;
  va_start( argumentlist, __fmt );
  vsprintf( aString, __fmt, argumentlist );
  va_end( argumentlist );
  drawString( x, y, aString );
}

#endif

// ----------------

// draw a vertical line

void drawVLine( word x0, word y0, word y1, byte colour )
{
  signed int pos  = 0;
  signed int temp = 0;
  if ( y0 > y1 )
  {
    word temp = y0; y0 = y1; y1 = temp;
  }
  while ( y1 > ( y0 - 1 ) )
  {
    setPixel( x0, y1, colour );
    y1--;
  }
}

// draw a horizontal line

void drawHLine( word x0, word y0, word x1, byte colour )
{
  signed int pos = 0;
  signed int temp = 0;
  if ( x0 > x1 )
  {
    word temp = x0; x0 = x1; x1 = temp;
  }
  while ( x1 > ( x0 - 1 ) )
  {
    setPixel( x1, y0, colour );
    x1--;
  }
}

// draw a line

void drawLine( word x0, word y0, word x1, word y1, byte colour )
{
  signed int dx       = 0;
  signed int dy       = 0;
  signed int stepx    = 0;
  signed int stepy    = 0;
  signed int fraction = 0;

  if ( x0 == x1 )
  {
    if ( y0 == y1 )
      setPixel( x0, y0, colour );
    else
      drawVLine( x0, y0, y1, colour );
    return;
  }
  if ( y0 == y1 )
  {
    drawHLine( x0, y0, x1, colour );
    return;
  }

  dy = ( y1 - y0 );
  dx = ( x1 - x0 );
  if ( dy < 0 )
  {
    dy    = -dy;
    stepy = -1;
  }
  else
  {
    stepy = 1;
  }
  if ( dx < 0 )
  {
    dx    = -dx;
    stepx = -1;
  }
  else
  {
    stepx = 1;
  }

  dx <<= 1;
  dy <<= 1;

  setPixel( x0, y0, colour );

  if ( dx > dy )
  {
    fraction = ( dy - ( dx >> 1 ) );
    while ( x0 != x1 )
    {
      if ( fraction >= 0 )
      {
        y0       += stepy;
        fraction -= dx;
      }
      x0       += stepx;
      fraction += dy;

      setPixel( x0, y0, colour );
    }
  }
  else
  {
    fraction = ( dx - ( dy >> 1 ) );
    while ( y0 != y1 )
    {
      if ( fraction >= 0 )
      {
        x0       += stepx;
        fraction -= dy;
      }
      y0       += stepy;
      fraction += dx;

      setPixel( x0, y0, colour );
    }
  }
}

// draw a rectangle
//   if fill > 0 then fill the rectangle

void drawRectangle( word x0, word y0, word x1, word y1, byte colour, byte fill )
{
  word xmin = 0;
  word xmax = 0;
  word ymin = 0;
  word ymax = 0;

  if ( x0 < x1 )
  {
    xmin = x0;
    xmax = x1;
  }
  else
  {
    xmin = x1;
    xmax = x0;
  }
  if ( y0 < y1 )
  {
    ymin = y0;
    ymax = y1;
  }
  else
  {
    ymin = y1;
    ymax = y0;
  }

  if ( x0 == x1 )
  {
    if ( y0 == y1 )
      setPixel( x0, y0, colour );
    else
      drawVLine( x0, y0, y1, colour );
    return;
  }
  if ( y0 == y1 )
  {
    drawHLine( x0, y0, x1, colour );
    return;
  }

  if ( xmax == ( xmin + 1 ) )
  {
    if ( ymax == ( ymin + 1 ) )
    {
      setPixel( x0, y0, colour );
      setPixel( x0, y1, colour );
      setPixel( x1, y0, colour );
      setPixel( x1, y1, colour );
    }
    else
    {
      drawVLine( xmin, ymin, ymax, colour );
      drawVLine( xmax, ymin, ymax, colour );
    }
    return;
  }
  if ( ymax == ( ymin + 1 ) )
  {
    drawHLine( xmin, ymin, xmax, colour );
    drawHLine( xmin, ymax, xmax, colour );
    return;
  }

  // filled rectangle
  if ( fill != 0 )
  {
    for ( ; xmin <= xmax; ++xmin )
    {
      drawVLine( xmin, ymin, ymax, colour );
    }
  }
  // rectangle (not filled)
  else
  {
    drawVLine( xmin,   ymin, ymax,   colour );  // left side
    drawVLine( xmax,   ymin, ymax,   colour );  // right side
    drawHLine( xmin+1, ymin, xmax-1, colour );  // bottom side
    drawHLine( xmin+1, ymax, xmax-1, colour );  // top side
  }
}

// --------

void drawCircle( int xc, int yc, int radius, byte colour)
{
  // Bresenham's circle drawing algorithm
  int x = -radius;
  int y = 0;
  int error = 2 - 2*radius;

  while ( x < 0 )
  {
    setPixel( xc-x, yc+y, colour);
    setPixel( xc-y, yc-x, colour);
    setPixel( xc+x, yc-y, colour);
    setPixel( xc+y, yc+x, colour);
    radius = error;
    if ( radius <= y ) error += ++y*2 + 1;
    if ( radius >  x || error > y ) error += ++x*2 + 1;
  }
}

void drawFilledCircle( int xc, int yc, int radius, byte colour )
{
  // Bresenham's circle drawing algorithm, filling with vertical line segments to/from origin
  int x = -radius;
  int y = 0;
  int error = 2 - 2*radius;

  while ( x < 0 )
  {
    drawVLine( xc-x, yc,   yc+y, colour );
    drawVLine( xc-y, yc,   yc-x, colour );
    drawVLine( xc+x, yc-y, yc,   colour );
    drawVLine( xc+y, yc+x, yc,   colour );
    radius = error;
    if ( radius <= y ) error += ++y*2 + 1;
    if ( radius >  x || error > y ) error += ++x*2 + 1;
  }
}

// --------

#ifdef _USE_ADAFRUIT

// draw a circle
//   if fill > 0 then fill the circle

void drawCircle( int xc, int yc, int r, byte colour )
{
  int f    =  1 - r;
  int ddFx =  1;
  int ddFy = -2 * r;
  int x    =  0;
  int y    =  r;

  setPixel( xc,     yc + r, colour );
  setPixel( xc,     yc - r, colour );
  setPixel( xc + r, yc,     colour );
  setPixel( xc - r, yc,     colour );

  while ( x < y )
  {
    if ( f >= 0 )
    {
      y--;
      ddFy += 2;
      f    += ddFy;
    }
    x++;
    ddFx += 2;
    f    += ddFx;

    setPixel( xc + x, yc + y, colour );
    setPixel( xc - x, yc + y, colour );
    setPixel( xc + x, yc - y, colour );
    setPixel( xc - x, yc - y, colour );
    setPixel( xc + y, yc + x, colour );
    setPixel( xc - y, yc + x, colour );
    setPixel( xc + y, yc - x, colour );
    setPixel( xc - y, yc - x, colour );
  }
}

void drawFilledCircle( int xc, int yc, int r, byte colour )
{
  int f     =  1 - r;
  int ddF_x =  1;
  int ddF_y = -2 * r;
  int x1    =  0;
  int y1    =  r;
  int x0    =  x1;
  int y0    =  y1;

  drawVLine( xc, yc - r, 2 * r + 1, colour );

  while ( x1 < y1 )
  {
    if ( f >= 0 )
    {
      y1--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x1++;
    ddF_x += 2;
    f     += ddF_x;

    // avoid double-drawing certain lines
    if ( x1 < (y1 + 1) )
    {
      drawVLine( xc+x1, yc-y1, 2*y1+1, colour );
      drawVLine( xc-x1, yc-y1, 2*y1+1, colour );
    }
    if ( y1 != y0 )
    {
      drawVLine( xc+y0, yc-x0, 2*x0+1, colour );
      drawVLine( xc-y0, yc-x0, 2*x0+1, colour );
      y0 = y1;
    }
    x0 = x1;
  }
}

#endif

// ----------------

void textSamples( void )
{
  for ( byte i = 0; i < 20; i++ )
  {
    drawChar( i*6, 0, (0x21+i) );
  }
  drawString( 4, 8, "SSD1309" );
  drawString( 8, 16, "2020-02-21" );
  for ( byte i = 0; i < 20; i++ )
  {
    drawChar( 12+i*6, 32, (0xA1+i) );
  }
}

void text2Samples( void )
{
  for ( byte i = 0; i < 20; i++ )
  {
    drawGraphic( i*6, 0, (0x04+i) );
  }
  for ( byte i = 0; i < 20; i++ )
  {
    drawGraphic( i*6, 16, (0xB0+i) );
  }
}

void checkerBoard( void )
{
  // creates a full screen checkerboard
  byte i, j, colour;
  colour = 0x00;
  for ( i = 0; i < 8; i++ )
  {
    colour = ~colour;
    setPage( i );
    setColumn( 0 );
    for ( j = 0; j < 128; j++ )
    {
      if ( j % 8 == 0 )
      {
        colour = ~colour;
      }
      writeData( colour );
    }
  }
}

// Demos read abilities.
// Only functional in parallel.
// Writes one page of squares, then reads the data and writes it four pages up.

byte readArray[128] = {};

void readDemo( void )
{
  byte i, j,
       colour = 0x00;

  // clear the display
  fillRAM( 0x00 );

  // draw 4 pages of checkerboard
  for ( i = 0; i < 4; i++ )
  {
    colour = ~colour;
    setPage( i );
    setColumn( 0 );
    for ( j = 0; j < 128; j++ )
    {
      if ( j % 8 == 0 )
      {
        colour = ~colour;
      }
      writeData( colour );
    }
  }

  drawString(  6,  8, "S S D 1 3 0 9" );
  drawString( 12, 16, "1-bit  O L E D" );

  delay( 500 );

  // copy top 4 pages to bottom
  for ( i = 0; i < 4; i++ )
  {
    setPage( i );
    setColumn( 0 );
    for ( j = 0; j < 128; j++ )
    {
      readArray[j] = readData();
    }

    setPage( i+4 );
    setColumn( 0 );
    for ( j = 0; j < 128; j++ )
    {
      writeData( readArray[127-j] );  // reverse
    }
  }
}

void pixels( void )
{
  byte x, y,
       p = random( 128 ) + 16;

  for ( byte i = 0; i < p; i++ )
  {
    x = random( 128 );
    y = random( 64 );
    setPixel( x, y, 0x01 );
    delay( 2 );
  }
}

void drawLines( void )
{
  byte x0, y0,
       x1, y1,
       lines   = random( 16 ) + 4;

  for ( byte i = 0; i < lines; i++ )
  {
    x0 = random( 128 );
    y0 = random(  64 );
    x1 = random( 128 );
    y1 = random(  64 );
    drawLine( x0, y0, x1, y1, 0x01 );
    delay( 25 );
  }
}

void drawRectangles( void )
{
  byte x0, y0,
       x1, y1,
       rects   = random( 8 ) + 2;

  for ( byte i = 0; i < rects; i++ )
  {
    x0 = random( 128 );
    y0 = random(  64 );
    x1 = random( 128 );
    y1 = random(  64 );
    drawRectangle( x0, y0, x1, y1, 0x01, i%2 );
    delay( 25 );
  }
}

void drawCircles( void )
{
  byte xc, yc,
       r,
       circles = random( 8 ) + 2;

  for ( byte i = 0; i < circles; i++ )
  {
    xc = random( 64 ) + 32;
    yc = random( 32 ) + 16;
    r  = random( 22 ) +  2;
    if ( ( i%2 ) == 0 )
      drawCircle( xc, yc, r, 0x01 );
    else
      drawFilledCircle( xc, yc, r, 0x01 );
    delay( 25 );
  }
}

// ----------------

// Demos - readdemo only works for parallel interfaces

#define waittime  4000

void setup( void )
{
  // initialize ports/pins
  initIO();

  // debug console - uncomment the serial stuff if you want to debug
//  Serial.begin( 9600 );
//  Serial.println( "setup()" );

  // Fire up the OLED
//  Serial.println( "initDisplay()" );
  initDisplay();
  delay( 2000 );

  randomSeed( analogRead( A1 ) );
}

void loop( void )
{
//Serial.println( "loop()" );

  setPage( 0 );
  setColumn( 0 );

//Serial.println( "fillRAM()" );
  fillRAM( 0xFF );
  delay( waittime );
  fillRAM( 0x00 );

//Serial.println( "textSamples()" );
  textSamples();
  delay( waittime );
  fillRAM( 0x00 );

//Serial.println( "checkerboard()" );
  checkerBoard();
  delay( waittime );
  fillRAM( 0x00 );

//Serial.println( "readDemo()" );
  readDemo();
  delay( waittime );
  fillRAM( 0x00 );

//Serial.println( "pixels()" );
  pixels();
  delay( waittime );
  fillRAM( 0x00 );

//Serial.println( "text2Samples()" );
  text2Samples();
  delay( waittime );
  fillRAM( 0x00 );

//  Serial.println( "drawLines()" );
  drawLines();
  delay( waittime );
  fillRAM( 0x00 );

//Serial.println( "drawRectangles()" );
  drawRectangles();
  delay( waittime );
  fillRAM( 0x00 );

//Serial.println( "drawCircles()" );
  drawCircles();
  delay( waittime );
  fillRAM( 0x00 );

}

// ----------------

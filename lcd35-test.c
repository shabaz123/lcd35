/***************************
 * lcd35-test.c
 * 
 * Revision 0.1  - original code for 3-digit display - April 2016 shabaz
 * Revision 0.2  - converted for 3.5 digit display - June 2019 shabaz
 *
 ***************************/


#include <stdio.h>
#include "i2cfunc.h"

// we will use I2C2 which is enumerated as 1 on the BBB
#define I2CBUS 1

// set to 1 to print out the intermediate calculations
#define DBG_PRINT 0
// set to 1 to use the example values in the datasheet
// rather than the values retrieved via i2c
#define ALG_TEST 0

#define LCD_ADDR 0x38
// Four possible values for DEVICE_ADDRESS
// R1   R2   R3   R4   DEVICE_ADDRESS
// Fit       FIT       0x60
//      Fit  Fit       0x61
// Fit            Fit  0x62
//      Fit       Fit  0x63


// If your LCD has 4 full digits (i.e. 8.8.8.8), then set DISP_TYPE to 4 
// If the LCD has only 3 full digits (i.e. 1.8.8.8) then set to 3 
#define DISP_TYPE 3

// Commands (these can be 'continued' by setting the MSB
#define DEVICE_ADDR 0x60
#define BLINK_OFF 0x70
#define BANK_CMD 0x78

// BANK_CMD bit values
#define WR_BANK_A 0x00
#define WR_BANK_B 0x02
#define DISP_BANK_A 0x00
#define DISP_BANK_B 0x01

typedef struct char_prog_s
{
  unsigned char byte0;
  unsigned char byte1;
  unsigned char byte2;
} char_prog_t;

typedef struct ascii_s
{
  char_prog_t char0;
  char_prog_t char1;
  char_prog_t char2;
} ascii_t;

// bits for displaying numbers 0-9
const char bit_table[]=
{
  0xf6, 0xc0, 0x6e, 0xea, 0xd8, 0xba, 0xbe, 0xe0, 0xfe, 0xfa
};


unsigned char buf[10];
int handle;
  
char_prog_t*
char_prog(int idx, unsigned char c)
{
  ascii_t *ascii_ptr;
  char_prog_t *char_prog_ptr;


  ascii_ptr=(ascii_t *)bit_table;
  char_prog_ptr=(char_prog_t *)(&ascii_ptr[c-'0']);
  return(&char_prog_ptr[idx]);
}

void
print_line(char* line)
{
  char line_idx=0;
  char lcd_idx=0;
  char not_finished=1;
  char c;
  char_prog_t *progval;
  unsigned char byte0=0;
  unsigned char byte1=0;
  unsigned char byte2=0;
  unsigned char byte3=0;
  unsigned char byte4=0;

  while(not_finished)
  {
    c=line[line_idx];
    switch(c)
    {
      case '.':
        // handle decimal point
        if (lcd_idx==1)
        {
          byte1 |= 0x01;
        }
        else if (lcd_idx==2)
        {
          byte2 |= 0x01;
        } 
        else if (lcd_idx==3)
        {
          byte3 |= 0x01;
        }
        break;
      case ' ':
        lcd_idx++;
        if (lcd_idx>3)
        {
          not_finished=0;
        }
        break;
      case '\0':
        // line finished
        not_finished=0;
        break;
      default:
        if ((c<'0') || (c>'9'))
        {
          // we can't handle this char
        }
        else
        {
           if (lcd_idx==0)
           {
             if (DISP_TYPE==4)
             {
                byte1 |= bit_table[c-'0'];
             }
             else
             {
               if (c=='1')
               {
                  byte0 |= 0x80;
               }
               else
               {
                  // can't handle any other char
               }
             }
           }
           else if (lcd_idx==1)
           {
             byte2 |= bit_table[c-'0'];
           }
           else if (lcd_idx==2)
           {
             byte3 |= bit_table[c-'0'];
           }
           else if (lcd_idx==3)
           {
              byte4 |= bit_table[c-'0'];
           }
           lcd_idx++;
           if (lcd_idx>3)
           {
             // we're done
             not_finished=0;
           }
        }
        break;
    } // end switch(c)
    line_idx++;
  } // end while(not_finished)

  buf[5]=byte0;
  buf[6]=byte1;
  buf[7]=byte2;
  buf[8]=byte3;
  buf[9]=byte4;

  i2c_write(handle, buf, 10);
}


int
main(void)
{
  int i;
  double v;
  unsigned char text[6];
  char_prog_t *progval;
  unsigned char mode, device, bank, blinkmode;

  mode=0xc9; // Set static mode, display enabled, continuation enabled
  device=DEVICE_ADDR | 0x80; // Select the device, continuation enabled
  bank=BANK_CMD | 0x80 | WR_BANK_A | DISP_BANK_A; 
  blinkmode=BLINK_OFF | 0x80;
  buf[0]=mode;
  buf[1]=device;
  buf[2]=blinkmode;
  buf[3]=bank;
  buf[4]=0x00; // pointer

  handle=i2c_open(I2CBUS, 0x38);

  buf[5]=0xf1; // data
  buf[6]=0x80; // data
  buf[7]=0x02; // data

  progval=char_prog(0, '5');
  buf[5]=progval->byte0;
  buf[6]=progval->byte1;
  buf[7]=progval->byte2;

  //print_line(" . .");

  //delay_ms(999);  
  


  //for (i=0; i<1000; i++)
  for (v=9.0; v<=19.9; v=v+0.01)
  {
    //sprintf(text, "%3d", i);
    // sprintf(text, "%5.1f", v);
    sprintf(text, "%5.2f", v);
    print_line(text);
    //print_line("12.34");
    delay_ms(40);
  }

  delay_ms(500);
  print_line("   ");


  i2c_close(handle);
  

  return(0);
}



#include <stdio.h>
#include <stdint.h>
//#include "crc16.h" // C:\Program Files (x86)\Arduino\hardware\tools\avr\avr\include\avr\crc16.h

/*****************************************************************************/
/**
   @brief Perform CRC calculation function

   @param buffer
   @param buffer_length
   @return uint16_t
*/
uint16_t crc16Calculation(uint8_t *buffer, uint8_t buffer_length)
{
  uint8_t j;
  uint16_t crc;
  crc = 0xFFFF;

  while (buffer_length--)
  {
    crc = crc ^ *buffer++;
    for (j = 0; j < 8; j++)
    {
      if (crc & 0x0001)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc = crc >> 1;
    }
  }
  // return (crc << 8 | crc >> 8); // for no bytes swapped
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  return crc;
}

/*****************************************************************************/
/**
   @brief

   @param msg
   @param msg_length
   @return true
   @return false
*/
bool crc16CheckIntegrity(uint8_t *msg, uint8_t msg_length)
{
  bool integrity_status = false;
  uint16_t crc_calculated;
  uint16_t crc_received;

  crc_calculated = crc16Calculation(msg, msg_length - 2);
  crc_received = (msg[msg_length - 2] << 8) | msg[msg_length - 1];

  /* Check CRC of msg */
  if (crc_calculated == crc_received)
  {
    integrity_status = true;
  }

  return integrity_status;
}

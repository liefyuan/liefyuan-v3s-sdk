#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#define TMP_EEPROM_DEV "/tmp/tftp/eeprom.conf"
#define EEPROM_DEV "/dev/eeprom_backboard"
#define EEPROM_SIZE 512


static void print_tip(void)
{
   printf("Tips:\n");
   printf("p   ---print eeprom (/dev/eeprom_backboard)\n");
   printf("c   ---clean eeprom (/dev/eeprom_backboard)\n");
   printf("t   ---print eeprom (/tmp/tftp/eeprom.conf)\n");
   printf("w [start_Byte(D)] [value1(H)] [value2(H)] ...   ---write eeprom from start_Byte(0-512)\n");
   printf("                                                ---like (./eeprom w 132 ff ee dd cc bb 33)\n");
   printf("mac_eth1    ---buff[138 - 143] like (./eeprom mac_eth1 00 11 22 33 44 55)\n");
   printf("#mac_inband  ---buff[132 - 137] like (./eeprom mac_inband 00 11 22 33 44 55)\n");
   printf("#mac_ethoam  ---buff[144 - 149] like (./eeprom mac_ethoam 00 11 22 33 44 55)\n");
}

static int eeprom_print(void)
{
   int fd = open(EEPROM_DEV, O_RDONLY);
   if (fd < 0) {
      printf("%s error.\n", EEPROM_DEV);
      return -1;
   }

   int cnt, i;
   unsigned char buff_r[EEPROM_SIZE];
   cnt = read(fd, buff_r, EEPROM_SIZE);
   printf("read OK:%d Byte\n", cnt);
   printf("Byte:   0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15\n");
   printf("  0: ");
   for (i = 0; i < EEPROM_SIZE; i++) {
      printf("0x%2x ", buff_r[i]);
      if ((i % 16) == 15) {
         printf("\n");
         printf("%3d: ", (i / 16 + 1));
      }
   }
   printf("\n");
   return 0;
}

static int tmp_eeprom_print(void)
{
   int fd = open(TMP_EEPROM_DEV, O_RDONLY);
   if (fd < 0) {
      printf("%s error.\n", TMP_EEPROM_DEV);
      return -1;
   }

   int cnt, i;
   unsigned char buff_r[EEPROM_SIZE];
   cnt = read(fd, buff_r, EEPROM_SIZE);
   printf("Byte:   0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15\n");
   printf("read OK:%d Byte\n", cnt);
   printf("  0: ");
   for (i = 0; i < EEPROM_SIZE; i++) {
      printf("0x%2x ", buff_r[i]);
      if ((i % 16) == 15) {
         printf("\n");
         printf("%3d: ", (i / 16 + 1));
      }
   }
   printf("\n");
   return 0;
}

static int eeprom_clean(unsigned char buff)
{
   int fd = open(EEPROM_DEV, O_RDWR);
   if (fd < 0) {
      printf("%s error.\n", EEPROM_DEV);
      return -1;
   }

   int cnt, i;
   unsigned char buff_clean[EEPROM_SIZE];
   memset(buff_clean, buff, EEPROM_SIZE);
   cnt = write(fd, buff_clean, EEPROM_SIZE);
   if (cnt != EEPROM_SIZE) {
      printf("clean error\n");
   } else printf("clean OK!\n");
   return 0;
}

static int eeprom_write(int start_Byte, char size, char *buff_write)
{
   if (start_Byte < 0) {
      printf("ERROR: start_Byte %d < 0, please write [start_Byte(D)] again\n", start_Byte);
      return -1;
   }
   if (!size) {
      printf("ERROR: please write [value(H)] ---like (./eeprom w 132 ff ee dd cc bb 33)\n");
      return -1;
   }
   if ((size + start_Byte) > EEPROM_SIZE) {
      printf("ERROR: too large to eeprom's size\n");
      return -1;
   }

   int fd = open(EEPROM_DEV, O_RDWR);
   if (fd < 0) {
      printf("%s error.\n", EEPROM_DEV);
      return -1;
   }

   int cnt, i;
   unsigned char buff[size];

   for (i = 0; i < size; i++) {
      buff[i] = *buff_write;
      printf("buff[%d] = 0x%x ", start_Byte + i, *buff_write);
      *buff_write++;
   }
   printf("\n");
   printf("write size = %d\n", size);
   lseek(fd, start_Byte, SEEK_SET);
   cnt = write(fd, buff, size);
   if (cnt != size) {
      printf("ERROR: write error!\n");
   }


   return 0;
}

int main(int argc, char *argv[])
{
   const char *str_cmd = NULL;
   char size;
   char buff_write[512] = {0};
   int start_Byte;
   int i = 0;
   unsigned char clean_buff = 0;

   if (argc > 1) {
      str_cmd = argv[1];
      switch (*str_cmd) {
         case 'p':
            eeprom_print();
            break;
         case 'c':
            clean_buff = strtoul(argv[2], NULL, 16);
            eeprom_clean(clean_buff);
            break;
         case 'w':
            if (argc >= 3) {
               start_Byte = strtoul(argv[2], NULL, 10);
               size = argc - 3;
               for (i = 0; i < size; i++){
                  buff_write[i] = strtoul(argv[i + 3], NULL, 16);
               }
               eeprom_write(start_Byte, size, buff_write);
            } else {
               printf("ERROR: please write [start_Byte(D)] ---like (./eeprom w 132 ff ee dd cc bb 33)\n");
            }
            break;
         case 'm':
            if (!strcmp(str_cmd, "mac_inband")) {
               if (argc != 8) {
                  printf("ERROR: need 6 value(H)\n");
               } else {
                  for (i = 0; i < 6; i++){
                     buff_write[i] = strtoul(argv[i + 2], NULL, 16);
                  }
                  eeprom_write(132, 6, buff_write);
               }
            } else if (!strcmp(str_cmd, "mac_eth1")) {
               if (argc != 8) {
                  printf("ERROR: need 6 value(H)\n");
               } else {
                  for (i = 0; i < 6; i++){
                     buff_write[i] = strtoul(argv[i + 2], NULL, 16);
                  }
                  eeprom_write(138, 6, buff_write);
               }
            } else if (!strcmp(str_cmd, "mac_ethoam")) {
               if (argc != 8) {
                  printf("ERROR: need 6 value(H)\n");
               } else {
                  for (i = 0; i < 6; i++){
                     buff_write[i] = strtoul(argv[i + 2], NULL, 16);
                  }
                  eeprom_write(144, 6, buff_write);
               }
            } else print_tip();
            break;
         case 't':
            tmp_eeprom_print();
            break;
         default:
            print_tip();
            break;
      }
   } else print_tip();

   return 0;
}


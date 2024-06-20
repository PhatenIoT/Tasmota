/*
  be_matter_module.c - implements the high level `matter` Berry module

  Copyright (C) 2023  Stephan Hadinger & Theo Arends

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/********************************************************************
 * Matter global module
 * 
 *******************************************************************/

#include "be_constobj.h"
#include "be_mapping.h"
#include <stdio.h>

// Matter logo
static const uint8_t MATTER_LOGO[] = 
  "<svg style='vertical-align:middle;' width='24' height='24' xmlns='http://www.w3.org/2000/svg' viewBox='100 100 240 240'>"
  "<defs><style>.cls-1{fill:none}.cls-2{fill:#FFFFFF;}</style></defs><rect class='cls-1' "
  "width='420' height='420'/><path class='cls-2' d='"
  "M167,156.88a71,71,0,0,0,32.1,14.73v-62.8l12.79-7.38,12.78,7.38v62.8a71.09,71.09,0,0,0,32.11-14.73"
  "L280,170.31a96.92,96.92,0,0,1-136.33,0Zm28.22,160.37A96.92,96.92,0,0,0,127,199.19v26.87a71.06,"
  "71.06,0,0,1,28.82,20.43l-54.39,31.4v14.77L114.22,300l54.38-31.4a71,71,0,0,1,3.29,35.17Zm101.5-"
  "118.06a96.93,96.93,0,0,0-68.16,118.06l23.27-13.44a71.1,71.1,0,0,1,3.29-35.17L309.46,300l12.78-"
  "7.38V277.89l-54.39-31.4a71.13,71.13,0,0,1,28.82-20.43Z'/></svg>";

// Matter stylesheet
static const uint8_t MATTER_STYLESHEET[] = 
  "<style>"
  ".bxm{height:14px;width:14px;display:inline-block;border:1px solid currentColor;background-color:var(--cl,#fff)}"
  ".ztdm td:not(:first-child){width:20px;font-size:70%}"
  ".ztdm td:last-child{width:45px}"
  ".ztdm .bt{margin-right:10px;}"
  ".htrm{line-height:20px}"
  "</style>";

static const uint8_t MATTER_ADD_ENDPOINT_HINTS_JS[] =
  "<script type='text/javascript'>"
  "function otm(arg_name,val){"
  "var s=eb(arg_name);"
  "s.placeholder=(val in hm)?hl[hm[val]]:\"\";"
  "s.title=s.placeholder;"
  "};"
  "</script>";

extern uint32_t matter_convert_seconds_to_dhm(uint32_t seconds,  char *unit, uint32_t *color, bbool days);

char* matter_seconds_to_dhm(int32_t seconds) {
  static const char empty_resp[] = "<td>&nbsp;</td>";
  static char res[64];                // static to allow returning to Berry
  char unit;
  uint32_t color;    // color of text

  if (seconds < 0) { return empty_resp; }       // no value

  uint32_t val = matter_convert_seconds_to_dhm(seconds, &unit, &color, bfalse);
  if (val < 100) {
    snprintf(res, sizeof(res), "<td style=\"color:#%02x%02x%02x\">&#x1F557;%02d%c</td>",
                                  (color & 0xFF0000) >> 16, (color & 0x00FF00) >> 8, (color & 0x0000FF),
                                  val, unit);
  } else {
    return empty_resp;
  }
  return res;
}
BE_FUNC_CTYPE_DECLARE(matter_seconds_to_dhm, "s", "i")

extern const bclass be_class_Matter_Counter;
extern const bclass be_class_Matter_Verhoeff;
extern const bclass be_class_Matter_QRCode;

#include "solidify/solidified_Matter_Module.h"

#include "../generate/be_matter_clusters.h"
#include "../generate/be_matter_opcodes.h"
#include "../generate/be_matter_vendors.h"

const char* matter_get_vendor_name(uint16_t id) {
  for (const matter_vendor_t * vnd = matter_Vendors; vnd->id != 0xFFFF; vnd++) {
    if (vnd->id == id) {
      return vnd->name;
    }
  }
  return NULL;
}
BE_FUNC_CTYPE_DECLARE(matter_get_vendor_name, "s", "i")

const char* matter_get_cluster_name(uint16_t cluster) {
  for (const matter_cluster_t * cl = matterAllClusters; cl->id != 0xFFFF; cl++) {
    if (cl->id == cluster) {
      return cl->name;
    }
  }
  return NULL;
}
BE_FUNC_CTYPE_DECLARE(matter_get_cluster_name, "s", "i")

const char* matter_get_opcode_name(uint16_t opcode) {
  for (const matter_opcode_t * op = matter_OpCodes; op->id != 0xFFFF; op++) {
    if (op->id == opcode) {
      return op->name;
    }
  }
  return NULL;
}
BE_FUNC_CTYPE_DECLARE(matter_get_opcode_name, "s", "i")

const char* matter_get_attribute_name(uint16_t cluster, uint16_t attribute) {
  if (attribute < 0xFFF0) {
    for (const matter_cluster_t * cl = matterAllClusters; cl->id != 0xFFFF; cl++) {
      if (cl->id == cluster) {
        for (const matter_attribute_t * at = cl->attributes; at->id != 0xFFFF; at++) {
          if (at->id == attribute) {
            return at->name;
          }
        }
      }
    }
  } else {
    for (const matter_attribute_t * at = matter_Attributes_Common; at->id != 0xFFFF; at++) {
      if (at->id == attribute) {
        return at->name;
      }
    }
  }
  return NULL;
}
BE_FUNC_CTYPE_DECLARE(matter_get_attribute_name, "s", "ii")

bbool matter_is_attribute_writable(uint16_t cluster, uint16_t attribute) {
  if (attribute < 0xFFF0) {
    for (const matter_cluster_t * cl = matterAllClusters; cl->id != 0xFFFF; cl++) {
      if (cl->id == cluster) {
        for (const matter_attribute_t * at = cl->attributes; at->id != 0xFFFF; at++) {
          if (at->id == attribute) {
            return (at->flags & 0x01) ? btrue : bfalse;
          }
        }
      }
    }
  }
  return bfalse;
}
BE_FUNC_CTYPE_DECLARE(matter_is_attribute_writable, "b", "ii")

bbool matter_is_attribute_reportable(uint16_t cluster, uint16_t attribute) {
  if (attribute < 0xFFF0) {
    for (const matter_cluster_t * cl = matterAllClusters; cl->id != 0xFFFF; cl++) {
      if (cl->id == cluster) {
        for (const matter_attribute_t * at = cl->attributes; at->id != 0xFFFF; at++) {
          if (at->id == attribute) {
            return (at->flags & 0x02) ? btrue : bfalse;
          }
        }
      }
    }
  } else {
    return btrue;
  }
  return bfalse;
}
BE_FUNC_CTYPE_DECLARE(matter_is_attribute_reportable, "b", "ii")

const char* matter_get_command_name(uint16_t cluster, uint16_t command) {
  for (const matter_cluster_t * cl = matterAllClusters; cl->id != 0xFFFF; cl++) {
    if (cl->id == cluster) {
      for (const matter_command_t * cmd = cl->commands; cmd->id != 0xFFFF; cmd++) {
        if (cmd->id == command) {
          return cmd->name;
        }
      }
    }
  }
  return NULL;
}
BE_FUNC_CTYPE_DECLARE(matter_get_command_name, "s", "ii")

// Convert an IP address from string to raw bytes
extern const void* matter_get_ip_bytes(const char* ip_str, size_t* ret_len);
BE_FUNC_CTYPE_DECLARE(matter_get_ip_bytes, "&", "s")

extern int matter_publish_command(bvm *vm);

#include "solidify/solidified_Matter_0_Inspect.h"

extern const bclass be_class_Matter_TLV;   // need to declare it upfront because of circular reference
#include "solidify/solidified_Matter_Path_0.h"
#include "solidify/solidified_Matter_Path_1_Generator.h"
#include "solidify/solidified_Matter_TLV.h"
#include "solidify/solidified_Matter_IM_Data.h"
#include "solidify/solidified_Matter_UDPServer.h"
#include "solidify/solidified_Matter_TCP_async.h"
#include "solidify/solidified_Matter_HTTP_async.h"
#include "solidify/solidified_Matter_HTTP_remote.h"
#include "solidify/solidified_Matter_Expirable.h"
#include "solidify/solidified_Matter_Fabric.h"
#include "solidify/solidified_Matter_Session.h"
#include "solidify/solidified_Matter_Session_Store.h"
#include "solidify/solidified_Matter_Commissioning_Data.h"
#include "solidify/solidified_Matter_Commissioning.h"
#include "solidify/solidified_Matter_Message.h"
#include "solidify/solidified_Matter_MessageHandler.h"
#include "solidify/solidified_Matter_IM_Message.h"
#include "solidify/solidified_Matter_IM_Subscription.h"
#include "solidify/solidified_Matter_IM.h"
#include "solidify/solidified_Matter_Control_Message.h"
#include "solidify/solidified_Matter_Plugin_0.h"
#include "solidify/solidified_Matter_Base38.h"
#include "solidify/solidified_Matter_UI.h"
#include "solidify/solidified_Matter_Profiler.h"

#include "../generate/be_matter_certs.h"

#include "solidify/solidified_Matter_Plugin_1_Root.h"
#include "solidify/solidified_Matter_Plugin_1_Aggregator.h"
#include "solidify/solidified_Matter_Plugin_1_Device.h"
#include "solidify/solidified_Matter_Plugin_2_OnOff.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_OnOff.h"
#include "solidify/solidified_Matter_Plugin_3_Light0.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_Light0.h"
#include "solidify/solidified_Matter_Plugin_2_Light1.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_Light1.h"
#include "solidify/solidified_Matter_Plugin_3_Light2.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_Light2.h"
#include "solidify/solidified_Matter_Plugin_3_Light3.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_Light3.h"
#include "solidify/solidified_Matter_Plugin_2_Shutter.h"
#include "solidify/solidified_Matter_Plugin_3_ShutterTilt.h"
#include "solidify/solidified_Matter_Plugin_2_Sensor.h"
#include "solidify/solidified_Matter_Plugin_3_Sensor_Pressure.h"
#include "solidify/solidified_Matter_Plugin_3_Sensor_Flow.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_Sensor_Pressure.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_Sensor_Flow.h"
#include "solidify/solidified_Matter_Plugin_3_Sensor_Temp.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_Sensor_Temp.h"
#include "solidify/solidified_Matter_Plugin_3_Sensor_Illuminance.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_Sensor_Illuminance.h"
#include "solidify/solidified_Matter_Plugin_3_Sensor_Humidity.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_Sensor_Humidity.h"
#include "solidify/solidified_Matter_Plugin_2_Sensor_Occupancy.h"
#include "solidify/solidified_Matter_Plugin_2_Sensor_OnOff.h"
#include "solidify/solidified_Matter_Plugin_2_Sensor_Contact.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_Sensor_Contact.h"
#include "solidify/solidified_Matter_Plugin_9_Virt_Sensor_Occupancy.h"
#include "solidify/solidified_Matter_Plugin_2_Bridge_HTTP.h"
#include "solidify/solidified_Matter_Plugin_4_Bridge_OnOff.h"
#include "solidify/solidified_Matter_Plugin_3_Bridge_Light0.h"
#include "solidify/solidified_Matter_Plugin_4_Bridge_Light1.h"
#include "solidify/solidified_Matter_Plugin_5_Bridge_Light2.h"
#include "solidify/solidified_Matter_Plugin_5_Bridge_Light3.h"
#include "solidify/solidified_Matter_Plugin_3_Bridge_Sensor.h"
#include "solidify/solidified_Matter_Plugin_4_Bridge_Sensor_Pressure.h"
#include "solidify/solidified_Matter_Plugin_4_Bridge_Sensor_Temp.h"
#include "solidify/solidified_Matter_Plugin_4_Bridge_Sensor_Illuminance.h"
#include "solidify/solidified_Matter_Plugin_4_Bridge_Sensor_Humidity.h"
#include "solidify/solidified_Matter_Plugin_3_Bridge_Sensor_Occupancy.h"
#include "solidify/solidified_Matter_Plugin_3_Bridge_Sensor_Contact.h"
#include "solidify/solidified_Matter_Plugin_4_Bridge_Sensor_Flow.h"
#include "solidify/solidified_Matter_Plugin_z_All.h"
#include "solidify/solidified_Matter_zz_Device.h"

#include "esp_efuse.h"
#include "esp_efuse_table.h"
#include <psa/crypto.h>
#include "mbedtls/aes.h"
#include "mbedtls/cipher.h"
#include "mbedtls/error.h"
#include "mbedtls/base64.h"
#include <mbedtls/bignum.h>
#include <mbedtls/ecp.h>
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/sha256.h"

#define POLY             0xEDB88320
#define KEY_LENGTH       16
#define IV_LENGTH        16
#define BLOCK_SIZE       16
#define AES_KEY_LENGTH   16 // AES-128

uint8_t kDevelopment_CD[300]={0};
uint8_t kDevelopment_PAI[465]={0};
uint8_t kDevelopment_DAC_Cert[600]={0};
uint8_t kDevelopment_DAC_Pri_key[32]={0};
uint8_t kDevelopment_DAC_Pub_key[65]={0};
unsigned int arrLen[5] = {0};
unsigned char fixed_key[KEY_LENGTH] = {'+', '-', '*', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '-','+'};
unsigned char decoded_iv[IV_LENGTH] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

// MAC地址转换为bytes函数
void mac_to_bytes(unsigned char* mac, unsigned char *fixed_key) {
    int i = 0;
    if (mac == NULL || fixed_key == NULL) {
        return; // 参数检查
    }

    // 存储转换后的MAC地址字符串，去掉冒号并转换为大写
    char mac_clean[13]; // 12位十六进制数加上结束符
    sprintf(mac_clean, "%02X%02X%02X%02X%02X%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    //printf("mac_to_bytes %s\n",mac_clean);
    // 将MAC地址字符串转换为ASCII码并填充到fixed_key中
    for (i = 0; i < strlen(mac_clean); i++) {
        // 将每个字符转换为其ASCII码值
        fixed_key[i] = mac_clean[i];
    }
    // 如果fixed_key未满，补0
    memset(fixed_key + i, 0, KEY_LENGTH - i);
}

// 初始化CRC值为0xFFFFFFFF，最后需要取反得到最终CRC值
uint32_t cal_crc32(uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc = crc ^ data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ POLY;
            } else {
                crc >>= 1;
            }
        }
    }
    // 最终CRC值需要取反
    return ~crc;
}

// 解密函数
int dac_decrypt(unsigned char *input, unsigned char **output, size_t input_len, size_t *output_len) {
    mbedtls_cipher_context_t cipher_ctx;
    size_t padded_output_len = input_len-4; 
    size_t padded_len = input_len-4;
    unsigned char *padded_output = NULL;
    size_t unpadded_len = 0;
    uint32_t crc32_bytes,new_crc32;
    int ret;
    crc32_bytes = (input[input_len-4]<<24)+(input[input_len-3]<<16)+(input[input_len-2]<<8)+input[input_len-1];
    // 初始化mbedtls库
    mbedtls_cipher_init(&cipher_ctx);
    // 设置cipher上下文
    mbedtls_cipher_setup(&cipher_ctx, mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CBC));
    mbedtls_cipher_setup(&cipher_ctx,mbedtls_cipher_info_from_values(MBEDTLS_CIPHER_ID_AES, 128,MBEDTLS_MODE_CBC));
    // 设置填充模式为PKCS7
    mbedtls_cipher_set_padding_mode(&cipher_ctx, MBEDTLS_PADDING_PKCS7);
    mbedtls_cipher_setkey(&cipher_ctx, fixed_key, AES_KEY_LENGTH*8, MBEDTLS_DECRYPT);
    mbedtls_cipher_set_iv(&cipher_ctx, decoded_iv, BLOCK_SIZE);
    mbedtls_cipher_reset(&cipher_ctx);

    // 分配输出缓冲区
    padded_output = malloc(padded_output_len);
    if (padded_output == NULL) {
        ret = -1; // 内存分配失败
        printf("dac_decrypt err 1\n");
        goto cleanup;
    }

    // 解密数据
    ret = mbedtls_cipher_update(&cipher_ctx, input, padded_len, padded_output, &padded_len);
    if (ret != 0) {
        free(padded_output);
        ret = -1;
        printf("dac_decrypt err 2\n");
        goto cleanup;
    }
    unpadded_len = padded_len;
	ret = mbedtls_cipher_finish(&cipher_ctx, padded_output + padded_len, &padded_len);
    if (ret != 0) {
        free(padded_output);
        ret = -1; 
        printf("dac_decrypt err 3\n");
        goto cleanup;
    }
	unpadded_len += padded_len;
    *output = padded_output;
    *output_len = unpadded_len;
    new_crc32 = cal_crc32(*output, *output_len);
    if(crc32_bytes == new_crc32)
    {
        printf("==== dac decrypt succ ====== \n");
    }
    else
    {
        printf("------ dac decrypt fail------ %lx %lx \n",crc32_bytes,new_crc32);
    }
cleanup:
    mbedtls_cipher_free(&cipher_ctx);
    return ret; // 返回错误代码
}

void my_set_fixed(uint8_t* key) 
{
    uint8_t mac[6];
    // 获取MAC地址
    esp_efuse_read_field_blob(ESP_EFUSE_MAC_FACTORY, mac, sizeof(mac) * 8);
    // 将MAC地址转换为fixed_key
    printf("MAC: " MACSTR "\n", MAC2STR(mac));
    mac_to_bytes(mac, fixed_key);
}

void convert_hex_to_bytes(const char* hex_data, unsigned char* bytes, int length) {
    const char* src;
    unsigned char* dst;
    int src_index, dst_index;

    src = hex_data;
    dst = bytes;
    src_index = 0;
    dst_index = 0;

    // 跳过空白字符
    while (src[src_index] == ' ') src_index++;

    // 解析十六进制数据直到遇到换行符或字符串结束
    while (dst_index < length && src[src_index] != '\n' && src[src_index] != '\0') 
    {
        if (src[src_index] >= '0' && src[src_index] <= '9') {
            dst[dst_index] = (src[src_index] - '0') << 4;
        }
        else if (src[src_index] >= 'A' && src[src_index] <= 'F') {
            dst[dst_index] = (src[src_index] - 'A' + 10) << 4;
        }
        else if (src[src_index] >= 'a' && src[src_index] <= 'f') {
            dst[dst_index] = (src[src_index] - 'a' + 10) << 4;
        }
        src_index++;

        if (src[src_index] >= '0' && src[src_index] <= '9') {
            dst[dst_index] |= src[src_index] - '0';
        }
        else if (src[src_index] >= 'A' && src[src_index] <= 'F') {
            dst[dst_index] |= src[src_index] - 'A' + 10;
        }
        else if (src[src_index] >= 'a' && src[src_index] <= 'f') {
            dst[dst_index] |= src[src_index] - 'a' + 10;
        }
        src_index++;
        dst_index++;
    }
    //printf("convert_hex_:%d\n",dst_index);
}

int parse_hex_data(const char* buf, int line_number, unsigned char* array, int* length) 
{
    const char* line_start;
    const char* line_end;
    int current_line = 0;
    int length_parsed;
    line_start = buf;
    while (current_line < line_number - 1 && line_start) {
        line_start = strchr(line_start, '\n');
        if (line_start) line_start++; 
        current_line++;
    }
    if (!line_start) return -1; 
    line_end = strchr(line_start, ' ');
    if (!line_end) return -1; 
    length_parsed = atoi(line_start);
    if (length_parsed <= 0 || length_parsed > 4096) return -1;
    convert_hex_to_bytes(line_end + 1, array, length_parsed);
    *length = length_parsed; 
    return 0; 
}

void read_config_files(unsigned char*cdData,unsigned char*PaiData,unsigned char*CertData,unsigned char*PriData,unsigned char*PubData,unsigned int*len)
{
  static unsigned char firstRead=0;
  unsigned char* encrypted_output;
  unsigned char buf[4096]={0};
  unsigned char arrData[640]={0};
  unsigned int lengh = 0;
  int bytes_read = 0;
  size_t decrypted_data_len = 0;
  const char *filename = "/paidac.txt";
  //printf("matter_ %x %x %x %x\n",addr[0],addr[1],addr[2],addr[3]);
  if(firstRead==0)
  {
    // 使用 open 打开文件
    my_set_fixed(fixed_key);
    FILE* fd = fopen(filename,"r");
    if (fd == NULL) 
    {
        printf("Error opening file\n");
    }
    else
    {
        memset(buf, 0, sizeof(buf));
        bytes_read = fread(buf, 1, sizeof(buf), fd);
        if (bytes_read == -1) 
        {
            printf("Error reading file\n");
            fclose(fd);
        }
        else
        {
            // 确保字符串以空字符结尾
            buf[bytes_read] = '\0';
            // 将读取的字节数设置为 length，且确保不会超过 buf 的大小
            lengh = (unsigned int)bytes_read;
            // 打印信息，确认读取的内容
            fclose(fd);
            for (unsigned int index = 1; index < 6; index++)
            {
                if (parse_hex_data((const char*)buf, index, arrData, &lengh) == 0) 
                {
                    len[index-1] = lengh;
                    printf("parsed %d %d \n", len[index-1], index-1);
                    if(index > 3)
                    {
                        dac_decrypt(arrData,&encrypted_output,len[index-1],&decrypted_data_len);
                        printf("Decrypted-data :%d %d \n",index,decrypted_data_len);
                        len[index-1] = decrypted_data_len;
                        for (int temp = 0; temp < decrypted_data_len; temp++) 
                        {
                            if(index == 5)
                            {
                                PubData[temp] = encrypted_output[temp];
                            }
                            else
                            {
                                PriData[temp] = encrypted_output[temp];
                            }
                        }
                    }

                    for (int i = 0; i < lengh; i++) 
                    {
                        if(index == 1)
                        cdData[i] = arrData[i];
                        else if(index == 2)
                        PaiData[i] = arrData[i];
                        else if(index == 3)
                        CertData[i] = arrData[i];
                        #if 0
                        else if(index == 4)
                        kDevelopment_DAC_Pri_key[i] = arrData[i];
                        else if(index == 5)
                        kDevelopment_DAC_Pub_key[i] = arrData[i];
                        //printf("%02X ", array[i]);
                        #endif
                    }
                }
                else
                {
                    perror("Failed to parse hex data");
                }
            }
        }
        firstRead = 1;
    }
  }
}

/*********************************************************************************************\
 * Get a bytes() object of the certificate DAC/PAI_Cert
\*********************************************************************************************/
#if 0
static int matter_return_static_bytes(bvm *vm, const uint8* addr, size_t len) {
  static unsigned char firstRead=0;
  unsigned char buf[4096]={0};
  unsigned char arrData[640]={0};
  unsigned int lengh = 0;
  int fd, bytes_read;
  const uint8* usraddr;
  size_t usrlen;
  const char *filename = "/paidac.txt";
  printf("matter_:%d %x %x %x %x\n",len,addr[0],addr[1],addr[2],addr[3]);
  if(firstRead==0)
  {
    // 使用 open 打开文件
    fd = open(filename, 0);
    if (fd == -1) 
    {
        printf("Error opening file\n");
    }
    else
    {
        bytes_read = read(fd, buf, sizeof(buf) - 1);
        if (bytes_read == -1) 
        {
            printf("Error reading file\n");
            close(fd);
        }
        else
        {
            // 确保字符串以空字符结尾
            buf[bytes_read] = '\0';
            // 将读取的字节数设置为 length，且确保不会超过 buf 的大小
            lengh = (unsigned int)bytes_read;
            // 打印信息，确认读取的内容
            close(fd);
            for (unsigned char index = 1; index < 6; index++)
            {
                if (parse_hex_data(buf, index, arrData, &lengh) == 0) 
                {
                    arrLen[index-1] = lengh;
                    printf("Successfully parsed %d %d \n", arrLen[index-1], index-1);
                    for (int i = 0; i < lengh; i++) 
                    {
                        if(index == 1)
                        kDevelopment_CD[i] = arrData[i];
                        else if(index == 2)
                        kDevelopment_PAI[i] = arrData[i];
                        else if(index == 3)
                        kDevelopment_DAC_Cert[i] = arrData[i];
                        else if(index == 4)
                        kDevelopment_DAC_Pri_key[i] = arrData[i];
                        else if(index == 5)
                        kDevelopment_DAC_Pub_key[i] = arrData[i];
                        //printf("%02X ", array[i]);
                    }
                }
                else
                {
                    perror("Failed to parse hex data");
                }
            }
        }
        firstRead = 1;
    }
  }
  // change data
  be_getbuiltin(vm, "bytes");
  if(len == 493)
  {
    usraddr = kDevelopment_DAC_Cert;
    usrlen = arrLen[2];
  }
  else if(len == 463)
  {
    usraddr = kDevelopment_PAI;
    usrlen = arrLen[1];

  }
  else if(len == 539)
  {
    usraddr = kDevelopment_CD;
    usrlen = arrLen[0];
  }
  else if(len == 32)
  {
    usraddr = kDevelopment_DAC_Pri_key;
    usrlen = arrLen[3];
  }
  else
  {
    usraddr = addr;
    usrlen = len;
  }
  printf("return_:%d %d %x %x %x %x\n",len,usrlen,usraddr[0],usraddr[1],usraddr[2],usraddr[3]);
  be_pushcomptr(vm, usraddr);
  be_pushint(vm, - usrlen);
  be_call(vm, 2);
  be_pop(vm, 2);
  be_return(vm);
}
#else
static int matter_return_static_bytes(bvm *vm, const uint8* addr, size_t len) {
  size_t usrlen;
  const uint8* usraddr;
  printf("matter_:%d %x %x %x %x\n",len,addr[0],addr[1],addr[2],addr[3]);
  read_config_files(kDevelopment_CD,kDevelopment_PAI,kDevelopment_DAC_Cert,kDevelopment_DAC_Pri_key,kDevelopment_DAC_Pub_key,arrLen);
  be_getbuiltin(vm, "bytes");
  if(len == 493)
  {
    usraddr = kDevelopment_DAC_Cert;
    usrlen = arrLen[2];
  }
  else if(len == 463)
  {
    usraddr = kDevelopment_PAI;
    usrlen = arrLen[1];

  }
  else if(len == 539)
  {
    usraddr = kDevelopment_CD;
    usrlen = arrLen[0];
  }
  else if(len == 32)
  {
    usraddr = kDevelopment_DAC_Pri_key;
    usrlen = arrLen[3];
  }
  else
  {
    usraddr = addr;
    usrlen = len;
  }
  printf("return_:%d %d %x %x %x %x\n",len,usrlen,usraddr[0],usraddr[1],usraddr[2],usraddr[3]);
  be_pushcomptr(vm, usraddr);
  be_pushint(vm, - usrlen);
  be_call(vm, 2);
  be_pop(vm, 2);
  be_return(vm);
}
#endif
static int matter_PAI_Cert_FFF1(bvm *vm) { return matter_return_static_bytes(vm, kDevelopmentPAI_Cert_FFF1, sizeof(kDevelopmentPAI_Cert_FFF1)); }
static int matter_PAI_Pub_FFF1(bvm *vm) { return matter_return_static_bytes(vm, kDevelopmentPAI_PublicKey_FFF1, sizeof(kDevelopmentPAI_PublicKey_FFF1)); }
static int matter_PAI_Priv_FFF1(bvm *vm) { return matter_return_static_bytes(vm, kDevelopmentPAI_PrivateKey_FFF1, sizeof(kDevelopmentPAI_PrivateKey_FFF1)); }
static int matter_DAC_Cert_FFF1_8000(bvm *vm) { return matter_return_static_bytes(vm, kDevelopmentDAC_Cert_FFF1_8000, sizeof(kDevelopmentDAC_Cert_FFF1_8000)); }
static int matter_DAC_Pub_FFF1_8000(bvm *vm) { return matter_return_static_bytes(vm, kDevelopmentDAC_PublicKey_FFF1_8000, sizeof(kDevelopmentDAC_PublicKey_FFF1_8000)); }
static int matter_DAC_Priv_FFF1_8000(bvm *vm) { return matter_return_static_bytes(vm, kDevelopmentDAC_PrivateKey_FFF1_8000, sizeof(kDevelopmentDAC_PrivateKey_FFF1_8000)); }
static int matter_CD_FFF1_8000(bvm *vm) { return matter_return_static_bytes(vm, kCdForAllExamples, sizeof(kCdForAllExamples)); }


#include "be_fixed_matter.h"

/* @const_object_info_begin

module matter (scope: global, strings: weak) {
  _LOGO, comptr(MATTER_LOGO)
  _STYLESHEET, comptr(MATTER_STYLESHEET)
  _ADD_ENDPOINT_JS, comptr(MATTER_ADD_ENDPOINT_HINTS_JS)
  MATTER_OPTION, int(151)       // SetOption151 enables Matter
  AGGREGATOR_ENDPOINT, int(0x0001)    // some controllers require aggregator to be endpoint 1
  START_ENDPOINT, int(0x0002)         // endpoint where to start devices
  seconds_to_dhm, ctype_func(matter_seconds_to_dhm)

  Verhoeff, class(be_class_Matter_Verhoeff)
  Counter, class(be_class_Matter_Counter)
  setmember, closure(matter_setmember_closure)
  member, closure(matter_member_closure)
  get_ip_bytes, ctype_func(matter_get_ip_bytes)
  publish_command, func(matter_publish_command)

  get_vendor_name, ctype_func(matter_get_vendor_name)
  get_cluster_name, ctype_func(matter_get_cluster_name)
  get_attribute_name, ctype_func(matter_get_attribute_name)
  is_attribute_writable, ctype_func(matter_is_attribute_writable)
  is_attribute_reportable, ctype_func(matter_is_attribute_reportable)
  get_command_name, ctype_func(matter_get_command_name)
  get_opcode_name, ctype_func(matter_get_opcode_name)
  TLV, class(be_class_Matter_TLV)
  sort, closure(matter_sort_closure)
  jitter, closure(matter_jitter_closure)
  inspect, closure(matter_inspect_closure)
  consolidate_clusters, closure(matter_consolidate_clusters_closure)
  UC_LIST, closure(matter_UC_LIST_closure)
  Profiler, class(be_class_Matter_Profiler)

  // Status codes
  SUCCESS, int(0x00)
  FAILURE, int(0x01)
  INVALID_SUBSCRIPTION, int(0x7D)
  UNSUPPORTED_ACCESS, int(0x7E)
  UNSUPPORTED_ENDPOINT, int(0x7F)
  INVALID_ACTION, int(0x80)
  UNSUPPORTED_COMMAND, int(0x81)
  INVALID_COMMAND, int(0x85)
  UNSUPPORTED_ATTRIBUTE, int(0x86)
  CONSTRAINT_ERROR, int(0x87)
  UNSUPPORTED_WRITE, int(0x88)
  RESOURCE_EXHAUSTED, int(0x89)
  NOT_FOUND, int(0x8B)
  UNREPORTABLE_ATTRIBUTE, int(0x8C)
  INVALID_DATA_TYPE, int(0x8D)
  UNSUPPORTED_READ, int(0x8F)
  DATA_VERSION_MISMATCH, int(0x92)
  TIMEOUT, int(0x94)
  UNSUPPORTED_NODE, int(0x9B)
  BUSY, int(0x9C)
  UNSUPPORTED_CLUSTER, int(0xC3)
  NO_UPSTREAM_SUBSCRIP­TION, int(0xC5)
  NEEDS_TIMED_INTERACTION, int(0xC6)
  UNSUPPORTED_EVENT, int(0xC7)
  PATHS_EXHAUSTED, int(0xC8)
  TIMED_REQUEST_MISMATCH, int(0xC9)
  FAILSAFE_REQUIRED, int(0xCA)

  // Matter_Data_IM classes
  AttributePathIB, class(be_class_Matter_AttributePathIB)
  ClusterPathIB, class(be_class_Matter_ClusterPathIB)
  DataVersionFilterIB, class(be_class_Matter_DataVersionFilterIB)
  AttributeDataIB, class(be_class_Matter_AttributeDataIB)
  AttributeReportIB, class(be_class_Matter_AttributeReportIB)
  EventFilterIB, class(be_class_Matter_EventFilterIB)
  EventPathIB, class(be_class_Matter_EventPathIB)
  EventDataIB, class(be_class_Matter_EventDataIB)
  EventReportIB, class(be_class_Matter_EventReportIB)
  CommandPathIB, class(be_class_Matter_CommandPathIB)
  CommandDataIB, class(be_class_Matter_CommandDataIB)
  InvokeResponseIB, class(be_class_Matter_InvokeResponseIB)
  CommandStatusIB, class(be_class_Matter_CommandStatusIB)
  EventStatusIB, class(be_class_Matter_EventStatusIB)
  AttributeStatusIB, class(be_class_Matter_AttributeStatusIB)
  StatusIB, class(be_class_Matter_StatusIB)
  StatusResponseMessage, class(be_class_Matter_StatusResponseMessage)
  ReadRequestMessage, class(be_class_Matter_ReadRequestMessage)
  ReadRequestMessage_solo, class(be_class_Matter_ReadRequestMessage_solo)
  ReportDataMessage, class(be_class_Matter_ReportDataMessage)
  SubscribeRequestMessage, class(be_class_Matter_SubscribeRequestMessage)
  SubscribeResponseMessage, class(be_class_Matter_SubscribeResponseMessage)
  WriteRequestMessage, class(be_class_Matter_WriteRequestMessage)
  WriteResponseMessage, class(be_class_Matter_WriteResponseMessage)
  TimedRequestMessage, class(be_class_Matter_TimedRequestMessage)
  InvokeRequestMessage, class(be_class_Matter_InvokeRequestMessage)
  InvokeRequestMessage_solo, class(be_class_Matter_InvokeRequestMessage_solo)
  InvokeResponseMessage, class(be_class_Matter_InvokeResponseMessage)

  // Matter Commisioning messages
  PBKDFParamRequest, class(be_class_Matter_PBKDFParamRequest)
  PBKDFParamResponse, class(be_class_Matter_PBKDFParamResponse)
  Pake1, class(be_class_Matter_Pake1)
  Pake2, class(be_class_Matter_Pake2)
  Pake3, class(be_class_Matter_Pake3)
  Sigma1, class(be_class_Matter_Sigma1)
  Sigma2, class(be_class_Matter_Sigma2)
  Sigma2Resume, class(be_class_Matter_Sigma2Resume)
  Sigma3, class(be_class_Matter_Sigma3)

  Commisioning_Context, class(be_class_Matter_Commisioning_Context)

  // UDP Server
  UDPPacket_sent, class(be_class_Matter_UDPPacket_sent)
  UDPServer, class(be_class_Matter_UDPServer)
  TCP_async, class(be_class_Matter_TCP_async)
  HTTP_async, class(be_class_Matter_HTTP_async)
  HTTP_remote, class(be_class_Matter_HTTP_remote)

  // Expirable
  Expirable, class(be_class_Matter_Expirable)
  Expirable_list, class(be_class_Matter_Expirable_list)

  // Sessions
  Fabric, class(be_class_Matter_Fabric)
  Session, class(be_class_Matter_Session)
  Session_Store, class(be_class_Matter_Session_Store)

  // Message Handler
  Frame, class(be_class_Matter_Frame)
  MessageHandler, class(be_class_Matter_MessageHandler)

  // Interation Model
  Path, class(be_class_Matter_Path)
  PathGenerator, class(be_class_Matter_PathGenerator)
  IM_Status, class(be_class_Matter_IM_Status)
  IM_InvokeResponse, class(be_class_Matter_IM_InvokeResponse)
  IM_WriteResponse, class(be_class_Matter_IM_WriteResponse)
  IM_ReportData, class(be_class_Matter_IM_ReportData)
  IM_ReportDataSubscribed, class(be_class_Matter_IM_ReportDataSubscribed)
  IM_SubscribeResponse, class(be_class_Matter_IM_SubscribeResponse)
  IM_SubscribedHeartbeat, class(be_class_Matter_IM_SubscribedHeartbeat)
  IM_Subscription, class(be_class_Matter_IM_Subscription)
  IM_Subscription_Shop, class(be_class_Matter_IM_Subscription_Shop)
  IM, class(be_class_Matter_IM)
  Control_Message, class(be_class_Matter_Control_Message)
  UI, class(be_class_Matter_UI)

  // QR Code
  QRCode, class(be_class_Matter_QRCode)

  // Base38 for QR Code
  Base38, class(be_class_Matter_Base38)

  // Matter Device core class
  Device, class(be_class_Matter_Device)

  // credentials from example
  PAI_Cert_FFF1, func(matter_PAI_Cert_FFF1)
  PAI_Pub_FFF1, func(matter_PAI_Pub_FFF1)
  PAI_Priv_FFF1, func(matter_PAI_Priv_FFF1)
  DAC_Cert_FFF1_8000, func(matter_DAC_Cert_FFF1_8000)
  DAC_Pub_FFF1_8000, func(matter_DAC_Pub_FFF1_8000)
  DAC_Priv_FFF1_8000, func(matter_DAC_Priv_FFF1_8000)
  CD_FFF1_8000, func(matter_CD_FFF1_8000)               // Certification Declaration

  // Plugins - only the core classes, all others are taken from `matter_device.plugins_classes`
  Plugin_Root, class(be_class_Matter_Plugin_Root)       // Generic behavior common to all devices
  Plugin_Aggregator, class(be_class_Matter_Plugin_Aggregator) // Aggregator
  Plugin_Bridge_HTTP, class(be_class_Matter_Plugin_Bridge_HTTP)     // HTTP bridge superclass
}

@const_object_info_end */


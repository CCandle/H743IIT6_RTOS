#pragma once

#include "OS/TaskBase.hpp"

#include "FreeRTOS.h"
#include "task.h"
#include <cstdio>
#include <cstring>

#include "lwip/err.h"     // 错误代码
#include "lwip/init.h"    // LwIP 初始化
#include "lwip/ip_addr.h" // IP 地址处理
#include "lwip/netif.h"   // 网络接口
#include "lwip/opt.h"     // LwIP 配置选项
#include "lwip/pbuf.h"    // 数据包缓冲区
#include "lwip/tcpip.h"   // TCP/IP 栈 API
#include "lwip/udp.h"     // UDP 相关函数

// FreeRTOS 头文件
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

class UDPTask : public AXITask<UDPTask, 1024> {
public:
  // 任务主函数
  void Run() {
    struct udp_pcb* upcb;
    ip_addr_t dest_ipaddr;
    struct pbuf* p;
    char msg[] = "1 hello";
    uint16_t dest_port = 8080;

    // 创建UDP控制块
    upcb = udp_new();
    if (upcb == NULL) {
      // printf("Failed to create UDP PCB\n");
      vTaskDelete(NULL);
      return;
    }

    // 设置目标IP地址（根据你的网络配置）
    IP4_ADDR(&dest_ipaddr, 192, 168, 22, 1);

    while (1) {
      msg[0]++;
      msg[0] > '9' ? msg[0] = '1' : 0;
      // 创建数据缓冲区
      p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg), PBUF_RAM);
      if (p != NULL) {
        // 拷贝数据到缓冲区
        memcpy(p->payload, msg, strlen(msg));

        // 发送UDP数据包
        err_t err = udp_sendto(upcb, p, &dest_ipaddr, dest_port);
        if (err == ERR_OK) {
          // printf("UDP packet sent successfully\n");
        } else {
          // printf("Failed to send UDP packet: %d\n", err);
        }

        // 释放缓冲区
        pbuf_free(p);
      } else {
        // printf("Failed to allocate pbuf\n");
      }

      // 每隔2秒发送一次
      vTaskDelay(pdMS_TO_TICKS(2000));
    }

    // 清理（通常不会执行到这里）
    udp_remove(upcb);
  }
};
/*-
 * $Copyright$
-*/
#include <version.h>

#include <FreeRTOS.h> /* for vTaskStartScheduler */

#include <stm32f4/PwrViaSTM32F4.hpp>
#include <stm32f4/FlashViaSTM32F4.hpp>
#include <stm32f4/RccViaSTM32.hpp>
#include <stm32f4/ScbViaSTM32F4.hpp>
#include <stm32f4/NvicViaSTM32F4.hpp>

#include <gpio/GpioAccess.hpp>

#include <gpio/GpioEngine.hpp>
#include <gpio/GpioPin.hpp>

#include <uart/UartAccess.hpp>
#include <uart/UartDevice.hpp>

#include <tasks/Heartbeat.hpp>

/*******************************************************************************
 * System Devices
 ******************************************************************************/
static const constexpr devices::PllConfigurationValuesT<devices::Stm32F407xx> pllCfgValues = {
    .m_pllSource        = devices::Stm32F407xx::PllSource_t::e_PllSourceHSE,
    .m_hseSpeedInHz     = 8 * 1000 * 1000,
    .m_pllM             = 8,
    .m_pllN             = 336,
    .m_pllP             = devices::Stm32F407xx::PllP_t::e_PllP_Div2,
    .m_pllQ             = devices::Stm32F407xx::PllQ_t::e_PllQ_Div7,
    .m_sysclkSource     = devices::Stm32F407xx::SysclkSource_t::e_SysclkPLL,
    .m_ahbPrescaler     = devices::Stm32F407xx::AHBPrescaler_t::e_AHBPrescaler_None,
    .m_apb1Prescaler    = devices::Stm32F407xx::APBPrescaler_t::e_APBPrescaler_Div4,
    .m_apb2Prescaler    = devices::Stm32F407xx::APBPrescaler_t::e_APBPrescaler_Div2
};

static const constexpr devices::PllConfigurationInterfaceT<decltype(pllCfgValues)> pllCfg(pllCfgValues);

static devices::PwrViaSTM32F4           pwr(PWR);
static devices::FlashViaSTM32F4         flash(FLASH);
static devices::RccViaSTM32F4           rcc(RCC, pllCfg, flash, pwr);
static devices::ScbViaSTM32F4           scb(SCB);
static devices::NvicViaSTM32F4          nvic(NVIC, scb);

/*******************************************************************************
 * GPIO Engine Handlers 
 ******************************************************************************/
static gpio::GpioAccessViaSTM32F4_GpioA gpio_A(rcc);
static gpio::GpioEngine                 gpio_engine_A(&gpio_A);

static gpio::GpioAccessViaSTM32F4_GpioC gpio_C(rcc);
static gpio::GpioEngine                 gpio_engine_C(&gpio_C);

static gpio::GpioAccessViaSTM32F4_GpioD gpio_D(rcc);
static gpio::GpioEngine                 gpio_engine_D(&gpio_D);

/*******************************************************************************
 * LEDs
 ******************************************************************************/
static gpio::PinT<decltype(gpio_engine_A)>  g_mco1(&gpio_engine_A, 8);
static gpio::PinT<decltype(gpio_engine_D)>  g_led_green(&gpio_engine_D, 12);

/*******************************************************************************
 * UART
 ******************************************************************************/
static gpio::PinT<decltype(gpio_engine_C)>  uart_tx(&gpio_engine_C, 6);
static gpio::PinT<decltype(gpio_engine_C)>  uart_rx(&gpio_engine_C, 7);
static uart::UartAccessSTM32F4_Uart6        uart_access(rcc, uart_rx, uart_tx);
uart::UartDevice                            g_uart(&uart_access);

/*******************************************************************************
 * Tasks
 ******************************************************************************/
static tasks::HeartbeatT<decltype(g_uart), decltype(g_led_green)>       heartbeat_gn("hrtbt_g", g_uart, g_led_green, 3, 500);

/*******************************************************************************
 *
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

extern char stext, etext;
extern char sdata, edata;
extern char sbss, ebss;
extern char bstack, estack;

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

const uint32_t SystemCoreClock = pllCfg.getSysclkSpeedInHz();

static_assert(pllCfg.isValid(pllCfg) == true,                       "PLL Configuration is not valid!");

static_assert(SystemCoreClock               == 168 * 1000 * 1000,   "Expected System Clock to be at 168 MHz!");
static_assert(pllCfg.getAhbSpeedInHz()      == 168 * 1000 * 1000,   "Expected AHB to be running at 168 MHz!");
static_assert(pllCfg.getApb1SpeedInHz()     ==  42 * 1000 * 1000,   "Expected APB1 to be running at 42 MHz!");
static_assert(pllCfg.getApb2SpeedInHz()     ==  84 * 1000 * 1000,   "Expected APB2 to be running at 84 MHz!");

/*******************************************************************************
 *
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

int
main(void) {
    g_led_green.enable(gpio::GpioAccessViaSTM32F4::e_Output, gpio::GpioAccessViaSTM32F4::e_None, gpio::GpioAccessViaSTM32F4::e_Gpio);

    rcc.setMCO<decltype(g_mco1), gpio::GpioAccessViaSTM32F4>(g_mco1, decltype(rcc)::MCO1Output_e::e_PLL, decltype(rcc)::MCOPrescaler_t::e_MCOPre_5);

    uart_access.setBaudRate(decltype(uart_access)::e_BaudRate_115200);

    g_uart.printf("Copyright (c) 2013-2020 Philip Schulz <phs@phisch.org>\r\n");
    g_uart.printf("All rights reserved.\r\n");
    g_uart.printf("\r\n");
    g_uart.printf("SW Version: %s\r\n", gSwVersionId);
    g_uart.printf("SW Build Timestamp: %s\r\n", gSwBuildTime);
    g_uart.printf("\r\n");
    g_uart.printf("Fixed Data: [0x0%x - 0x0%x]\t(%d Bytes total, %d Bytes used)\r\n",
      &gFixedDataBegin, &gFixedDataEnd, &gFixedDataEnd - &gFixedDataBegin, &gFixedDataUsed- &gFixedDataBegin);
    g_uart.printf("      Code: [0x0%x - 0x0%x]\t(%d Bytes)\r\n", &stext, &etext, &etext - &stext);
    g_uart.printf("      Data: [0x%x - 0x%x]\t(%d Bytes)\r\n", &sdata, &edata, &edata - &sdata);
    g_uart.printf("       BSS: [0x%x - 0x%x]\t(%d Bytes)\r\n", &sbss, &ebss, &ebss - &sbss);
    g_uart.printf(" Total RAM: [0x%x - 0x%x]\t(%d Bytes)\r\n", &sdata, &ebss, &ebss - &sdata);
    g_uart.printf("     Stack: [0x%x - 0x%x]\t(%d Bytes)\r\n", &bstack, &estack, &estack - &bstack);
    g_uart.printf("\r\n");

    const unsigned sysclk = pllCfg.getSysclkSpeedInHz() / 1000;
    const unsigned ahb    = pllCfg.getAhbSpeedInHz() / 1000;
    const unsigned apb1   = pllCfg.getApb1SpeedInHz() / 1000;
    const unsigned apb2   = pllCfg.getApb2SpeedInHz() / 1000;

    g_uart.printf("CPU running @ %d kHz\r\n", sysclk);
    g_uart.printf("        AHB @ %d kHz\r\n", ahb);
    g_uart.printf("       APB1 @ %d kHz\r\n", apb1);
    g_uart.printf("       APB2 @ %d kHz\r\n", apb2);
    g_uart.printf("\r\n");

    if (SysTick_Config(pllCfg.getSysclkSpeedInHz() / 1000)) {
        g_uart.printf("FATAL: Capture Error!\r\n");
        goto bad;
    }

    g_uart.printf("Starting FreeRTOS Scheduler...\r\n");
    xPortStartScheduler();

bad:
    g_uart.printf("FATAL ERROR!\r\n");
    while (1) ;

    return (0);
}

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

#ifndef MCP23S17_H
#define MCP23S17_H

#include <cstdint>
#include "hardware/spi.h"


class MCP23S17{
    public:
        // MCP23S17 registers (BANK=0)
        enum class Reg : uint8_t {
            IODIRA = 0x00, IODIRB = 0x01,
            IPOLA  = 0x02, IPOLB  = 0x03,
            GPINTENA=0x04, GPINTENB=0x05,
            DEFVALA =0x06, DEFVALB =0x07,
            INTCONA =0x08, INTCONB =0x09,
            IOCON   =0x0A, // mirror at 0x0B
            GPPUA  = 0x0C, GPPUB  = 0x0D,
            INTFA  = 0x0E, INTFB  = 0x0F,
            INTCAPA= 0x10, INTCAPB= 0x11,
            GPIOA  = 0x12, GPIOB  = 0x13,
            OLATA  = 0x14, OLATB  = 0x15
        };

        MCP23S17(spi_inst_t* spi, uint cs_gpio, uint8_t hw_addr=0);

        //low-level access
        void writeReg(Reg reg, uint8_t value);
        uint8_t readReg(Reg reg);

        //port-wide config
        void pinModeA(uint8_t dir_mask); // A-GPIO BUS 0xFF=input, 0x00=output
        void pinModeB(uint8_t dir_mask); // B-GPIO BUS 0xFF=input, 0x00=output
        void setPullupA(uint8_t m); // A-GPIO BUS 0xFF=enable_pullup, 0x00=disable_pullup
        void setPullupB(uint8_t m); // B-GPIO BUS 0xFF=enable_pullup, 0x00=disable_pullup
        void setPolarityA(uint8_t m); // A-GPIO BUS 0xFF=invert_input
        void setPolarityB(uint8_t m); // B-GPIO BUS 0xFF=invert_input

        //port-wide IO
        void writeA(uint8_t value); //write to OLATA
        void writeB(uint8_t value); //write to OLATB
        uint8_t readA(); //read to GPIOA
        uint8_t readB(); //read to GPIOB

        //per-pin IO
        void pinMode(char port, uint8_t pin, bool input);
        void digitalWrite(char port, uint8_t pin, bool input);
        bool digitalRead(char port, uint8_t pin);
        void setPullup(char port, uint8_t pin, bool enable);
        void setPolarity(char port, uint8_t pin, bool invert);

        //interupt controls
        void mirrorInterrupts(bool mirror);
        void setInterruptOpenDrain(bool od);
        void setInterruptPolarity(bool active_high);

        // interupt port-wide masks
        void interruptEnableA(uint8_t mask);
        void interruptEnableB(uint8_t mask);
        void interruptControlA(uint8_t mask);
        void interruptControlB(uint8_t mask);
        void interruptDefvalA(uint8_t mask);
        void interruptDefvalB(uint8_t mask);

        // interupt per-pin
        void interruptEnable(char port, uint8_t pin, bool enable);
        void interruptCompareToDefval(char port, uint8_t pin, bool use_defval);
        void interruptSetDefval(char port, uint8_t pin, bool level);
        
        //interrupt status
        uint8_t interruptFlags(char port);
        uint8_t interruptCapture(char port);

    private:
        spi_inst_t* spi_;
        uint cs_;
        uint8_t addr_;

        //opcodes
        uint8_t opcodeWrite() const {return uint8_t(0x40 | (addr_ << 1) | 0); }
        uint8_t opcodeRead()  const { return uint8_t(0x40 | (addr_ << 1) | 1); }

        //IOCON bit masks
        static constexpr uint8_t IOCON_MIRROR = 0x40;
        static constexpr uint8_t IOCON_ODR    = 0x04;
        static constexpr uint8_t IOCON_INTPOL = 0x02;
        
        //helper functions
        static inline uint8_t u8(Reg r) { return static_cast<uint8_t>(r); }
        void csLow() const;
        void csHigh() const;

        static uint8_t regGPIO(char p)   { return (p=='A'||p=='a') ? 0x12 : 0x13; }
        static uint8_t regOLAT(char p)   { return (p=='A'||p=='a') ? 0x14 : 0x15; }
        static uint8_t regIODIR(char p)  { return (p=='A'||p=='a') ? 0x00 : 0x01; }
        static uint8_t regGPPU(char p)   { return (p=='A'||p=='a') ? 0x0C : 0x0D; }
        static uint8_t regIPOL(char p)   { return (p=='A'||p=='a') ? 0x02 : 0x03; }
        static uint8_t regGPINTEN(char p){ return (p=='A'||p=='a') ? 0x04 : 0x05; }
        static uint8_t regDEFVAL(char p) { return (p=='A'||p=='a') ? 0x06 : 0x07; }
        static uint8_t regINTCON(char p) { return (p=='A'||p=='a') ? 0x08 : 0x09; }
        static uint8_t regINTF(char p)   { return (p=='A'||p=='a') ? 0x0E : 0x0F; }
        static uint8_t regINTCAP(char p) { return (p=='A'||p=='a') ? 0x10 : 0x11; }

        //read-modify-write utility
        void updateBit(char port, uint8_t reg, uint8_t pin, bool set);

};



#endif
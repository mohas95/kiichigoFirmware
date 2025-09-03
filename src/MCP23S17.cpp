#include "MCP23S17.h"
#include "pico/stdlib.h"


MCP23S17::MCP23S17(spi_inst_t* spi, uint cs_gpio, uint8_t hw_addr): spi_(spi),
                                                                    cs_(cs_gpio),
                                                                    addr_(uint8_t(hw_addr & 0x07)){

    gpio_init(cs_);
    gpio_set_dir(cs_, GPIO_OUT);
    gpio_put(cs_, 1);
}

//low-level access

void MCP23S17::csLow() const{gpio_put(cs_,0);}
void MCP23S17::csHigh() const{gpio_put(cs_,1);}

void MCP23S17::writeReg(Reg reg, uint8_t value){
    uint8_t buf[3] = {opcodeWrite(), u8(reg), value};
    csLow();
    spi_write_blocking(spi_,buf,3);
    csHigh();
}

uint8_t MCP23S17::readReg(Reg reg){

    uint8_t tx[3] = {opcodeRead(), u8(reg), 0x00};
    uint8_t rx[3];

    csLow();
    spi_write_read_blocking(spi_,tx,rx,3);
    csHigh();

    return rx[2]; 
}

//port-wide config

void MCP23S17::pinModeA(uint8_t dir_mask) { writeReg(Reg::IODIRA, dir_mask); }
void MCP23S17::pinModeB(uint8_t dir_mask) { writeReg(Reg::IODIRB, dir_mask); }
void MCP23S17::setPullupA(uint8_t m)      { writeReg(Reg::GPPUA,  m); }
void MCP23S17::setPullupB(uint8_t m)      { writeReg(Reg::GPPUB,  m); }
void MCP23S17::setPolarityA(uint8_t m)    { writeReg(Reg::IPOLA,  m); }
void MCP23S17::setPolarityB(uint8_t m)    { writeReg(Reg::IPOLB,  m); }

//port-wide IO
void MCP23S17::writeA(uint8_t value){writeReg(Reg::OLATA,value);} 
void MCP23S17::writeB(uint8_t value){writeReg(Reg::OLATB,value);} 
uint8_t MCP23S17::readA(){return readReg(Reg::GPIOA);} 
uint8_t MCP23S17::readB(){return readReg(Reg::GPIOB);}

//per-pin IO

void MCP23S17::updateBit(char port, uint8_t reg, uint8_t pin, bool set) {
    pin &= 0x07;
    uint8_t v = readReg(static_cast<Reg>(reg));
    if (set) v |=  (1u << pin);
    else     v &= ~(1u << pin);
    writeReg(static_cast<Reg>(reg), v);
}

void MCP23S17::pinMode(char port, uint8_t pin, bool input){
    updateBit(port, regIODIR(port), pin, input);
}
void MCP23S17::digitalWrite(char port, uint8_t pin, bool input){
    updateBit(port, regOLAT(port), pin, input);
}
bool MCP23S17::digitalRead(char port, uint8_t pin){
    pin &= 0x07;
    uint8_t v = readReg(static_cast<Reg>(regGPIO(port)));
    return (v >> pin) & 0x01;
}
void MCP23S17::setPullup(char port, uint8_t pin, bool enable){
    updateBit(port, regGPPU(port), pin, enable);
}
void MCP23S17::setPolarity(char port, uint8_t pin, bool invert){
    updateBit(port, regIPOL(port), pin, invert);
}

//Interrupt controls
void MCP23S17::mirrorInterrupts(bool mirror){
    uint8_t v = readReg(Reg::IOCON);
    v = mirror ? (uint8_t)(v | IOCON_MIRROR) : (uint8_t)(v & ~IOCON_MIRROR);
    writeReg(Reg::IOCON, v);
}
void MCP23S17::setInterruptOpenDrain(bool od){
    uint8_t v = readReg(Reg::IOCON);
    v = od ? (uint8_t)(v | IOCON_ODR) : (uint8_t)(v & ~IOCON_ODR);
    writeReg(Reg::IOCON, v);
}
void MCP23S17::setInterruptPolarity(bool active_high){
    uint8_t v = readReg(Reg::IOCON);
    v = active_high ? (uint8_t)(v | IOCON_INTPOL) : (uint8_t)(v & ~IOCON_INTPOL);
    writeReg(Reg::IOCON, v);
}

// interrupt port wide mask

void MCP23S17::interruptEnableA(uint8_t mask){ writeReg(Reg::GPINTENA, mask); }
void MCP23S17::interruptEnableB(uint8_t mask){ writeReg(Reg::GPINTENB, mask); }
void MCP23S17::interruptControlA(uint8_t mask){ writeReg(Reg::INTCONA, mask); }
void MCP23S17::interruptControlB(uint8_t mask){ writeReg(Reg::INTCONB, mask); }
void MCP23S17::interruptDefvalA(uint8_t mask){ writeReg(Reg::DEFVALA, mask); }
void MCP23S17::interruptDefvalB(uint8_t mask){ writeReg(Reg::DEFVALB, mask); }


//interrupt per pin

void MCP23S17::interruptEnable(char p, uint8_t pin, bool en){
    updateBit(p, regGPINTEN(p), pin, en);
}

void MCP23S17::interruptCompareToDefval(char p, uint8_t pin, bool use_defval){
    updateBit(p, regINTCON(p), pin, use_defval); // 1=compare to DEFVAL, 0=compare to previous
}

void MCP23S17::interruptSetDefval(char p, uint8_t pin, bool level){
    updateBit(p, regDEFVAL(p), pin, level);
}

// Status
uint8_t MCP23S17::interruptFlags(char p){
    return readReg(static_cast<Reg>(regINTF(p)));
}

uint8_t MCP23S17::interruptCapture(char p){
    // Read INTCAPx; reading INTCAP or GPIO clears the interrupt
    return readReg(static_cast<Reg>(regINTCAP(p)));
}
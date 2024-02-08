

class TB67S128FTG {

    public:
        TB67S128FTG(unsigned int dirPin, 
                    unsigned int stepPin, 
                    unsigned int stbyPin, 
                    unsigned int mode0Pin, 
                    unsigned int mode1Pin, 
                    unsigned int mode2Pin ) : 
                    dirPin(dirPin), 
                    stepPin(stepPin), 
                    stbyPin(stbyPin), 
                    mode0Pin(mode0Pin), 
                    mode1Pin(mode1Pin), 
                    mode2Pin(mode2Pin){

                    }


    private:
        unsigned int dirPin, stepPin, stbyPin, mode0Pin, mode1Pin, mode2Pin;



}
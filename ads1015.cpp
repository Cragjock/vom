#include <chrono>		//chrono::milliseconds(1000); need scope chrono
#include <thread>
#include <vector>
#include <bitset>
#include "ads1015.h"
#include "myi2c.h"

using namespace std;

vector<string> vmux_type=
{
    "Ain_p=Ain0 & Ain_n=Ain1",
    "Ain_p=Ain0 & Ain_n=Ain3",
    "Ain_p=Ain1 & Ain_n=Ain3",
    "Ain_p=Ain2 & Ain_n=Ain3",
    "Ain_p=Ain0 & Ain_n=gnd",
    "Ain_p=Ain1 & Ain_n=gnd",
    "Ain_p=Ain2 & Ain_n=gnd",
    "Ain_p=Ain3 & Ain_n=gnd"
};

string mux_type[]=              /// char* mux_type[]=
{
    "Ain_p=Ain0 & Ain_n=Ain1",
    "Ain_p=Ain0 & Ain_n=Ain3",
    "Ain_p=Ain1 & Ain_n=Ain3",
    "Ain_p=Ain2 & Ain_n=Ain3",
    "Ain_p=Ain0 & Ain_n=gnd",
    "Ain_p=Ain1 & Ain_n=gnd",
    "Ain_p=Ain2 & Ain_n=gnd",
    "Ain_p=Ain3 & Ain_n=gnd"
};

/*** Samples per second ***/
unsigned int data_rates[8] = {128, 250, 490, 920, 1600, 2400, 3300, 3300};

/*** PGA fullscale voltages in mV ***/
unsigned int fullscale[8] = {6144, 4096, 2048, 1024, 512, 256, 256, 256 };

char* c_mode[]={"Continuous convert", "Single-shot convert"};

enum channel {MUX0, MUX1, MUX2, MUX3, MUX4, MUX5, MUX6, MUX7};
enum D_R {DR128SPS, DR250SPS, DR490SPS, DR920SPS, DR1600SPS, DR2400SPS, DR3300SPS, DR3301SPS};
enum FScale {mv6144, mv4096, mv2048, mv1024, mv512, mv256};

enum class datarates {DR128SPS, DR250SPS, DR490SPS, DR920SPS, DR1600SPS, DR2400SPS, DR3300SPS, DR3301SPS};
enum class gain {mv6144, mv4096, mv2048, mv1024, mv512, mv256};
//enum class mode {continous, single};
enum class update : int {PGA, MUX, OPS, DR};  /// for config reg update


myADS1015 ADC = {0x48, mv4096, 0, MUX0, DR250SPS};
myADS1015 arrADC[4]=  {
                    {0x48, mv4096, 0, MUX0, DR250SPS},
                    {0x48, mv2048, 1, MUX1, DR250SPS},
                    {0x48, mv1024, 1, MUX4, DR1600SPS},
                    {0x48, 0, 0, 0, 0},
                };

vector<double>mv_gain={3.0, 2.0, 1.0, 0.5, 0.25, 0.125};


/*************************************************/
 ads1015::ads1015()
 {
    myI2C= unique_ptr<I2CBus>(new I2CBus(1,0x48)); //myI2C = new I2CBus(1,0x48);
    ADS1015_Init();
 }

 /*************************************************/
 ads1015::ads1015(uint8_t bus, uint8_t address)
 {
    myI2C= unique_ptr<I2CBus>(new I2CBus(1,0x48));  //myI2C = new I2CBus(1,0x48);
    ADS1015_Init();
 }

/*************************************************/
 ads1015::~ads1015()
 {

 }

/*************************************************/
uint16_t ads1015::update_config_reg(update item, uint16_t cr_update)
{
    switch(item)
    {
        case update::PGA:
            {
                bitset<16> PGA_Mask(0x0E00);
                config_register = config_register & ~(PGA_Mask.to_ullong());
                config_register |= cr_update;
                break;
            }
        case update::MUX:
            {
                bitset<16> MUX_Mask(0x7000);
                config_register = config_register & ~(MUX_Mask.to_ullong());
                config_register |= cr_update;
                break;
            }
        case update::OPS:
            {
                bitset<16> OPS_Mask(0x0100);
                config_register = config_register & ~(OPS_Mask.to_ullong());
                config_register |= cr_update;
                break;
            }
        case update::DR:
            {
                bitset<16> DR_Mask(0x00e0);
                config_register = config_register & ~(DR_Mask.to_ullong());
                config_register |= cr_update;
                break;
            }
        default: config_register |= cr_update; break;
    }
}

/*************************************************/
uint16_t ads1015::read_config_reg()
{
    uint16_t result = 67;   //BS number to start
    uint16_t resultswap = 17;

    result = myI2C->myI2C_read_word_data(Config_Reg);
    printf("read config register: %x \n", result);

    resultswap = myI2C->device_read_swap(Config_Reg);
    printf("read swapped config register: %x \n", resultswap);

    uint16_t index_data_rate= (resultswap & 0x0e0) >> CR_DR0;
    uint16_t index_MUX= (resultswap & 0x7000) >> CR_MUX0;
    uint16_t index_PGA= (resultswap &0x0E00 ) >> CR_PGA0;

    arrADC[3].data_rate = (resultswap & 0x0e0) >> CR_DR0;
    arrADC[3].Mux = (resultswap & 0x7000) >> CR_MUX0;
    arrADC[3].PGA = (resultswap &0x0E00 ) >> CR_PGA0;

    printf("current config reg is: %x\ndata rate is: %x\nmux is: %x\nPGA is: %x\n",
                    resultswap,
                    arrADC[3].data_rate,
                    arrADC[3].Mux,
                    arrADC[3].PGA);

    cout<<" =MUX number: "<<mux_type[index_MUX].c_str()<<endl;
    cout<<" =PGA number: "<<fullscale[index_PGA]<<endl;
    cout<<" =DR  number: "<<data_rates[index_data_rate]<<endl;

    MV = arrADC[3].PGA;

    return resultswap;
}

/**************************************
    Initialize ADC
**************************************/
/*************************************************/
int ads1015::ADS1015_Init()
{
    uint16_t init_config_reg = mux_single_1 | PGA_4096 | DR_250sps | MODE_CONTINUOUS | COMP_QUE_DISABLE;  /// should be 0x4223
    config_register = init_config_reg;
    myI2C->device_write_swap(Config_Reg,init_config_reg);

    current_PGA = (init_config_reg &0x0E00);
    current_MUX = (init_config_reg &0x7000);
    current_DR = (init_config_reg &0x0e0);

    switch(current_PGA)
    {
        case PGA_6144: MV = static_cast<uint8_t>(gain::mv6144); break;
        case PGA_4096: MV = static_cast<uint8_t>(gain::mv4096); break;
        case PGA_2048: MV = static_cast<uint8_t>(gain::mv2048); break;
        case PGA_1024: MV = static_cast<uint8_t>(gain::mv1024); break;
        case PGA_0512: MV = static_cast<uint8_t>(gain::mv512); break;
        case PGA_0256: MV = static_cast<uint8_t>(gain::mv256); break;
        default: MV = static_cast<uint8_t>(gain::mv4096); break;
    }
    MV = static_cast<uint8_t>(gain::mv4096);
    convert_type = mode::continous;
    ///update_config_reg(update::OPS, MODE_SINGLE_SHOT);

    return 1;
}

/*************************************************/
void ads1015::set_mode(mode op_status)
{
    if(op_status == mode::continous )
    {
        convert_type = mode::continous;
        update_config_reg(update::OPS, MODE_CONTINUOUS);
    }
    else if(op_status == mode::single )
    {
        convert_type = mode::single;
        update_config_reg(update::OPS, MODE_SINGLE_SHOT);
    }
}

/*************************************************/
/**< KEEP THIS ***************/
/*************************************************/
float ads1015::read_conversion()
{
    cout<<"~~~ Start conversion ~~~"<<endl;
    if(convert_type == mode::single)
        {
            cout<<"~~~~start single conversion "<<endl;
            bitset<16> START_Mask(0x8000);
            config_register = config_register | (START_Mask.to_ullong()); /// set bit 15 = 1
            myI2C->device_write_swap(Config_Reg,config_register);
            this_thread::sleep_for(chrono::milliseconds(5));
        }
    int16_t result = 0x1234; // due to __s32 i2c_smbus_read_word_data(int file, __u8 command)
    int16_t result_sw = 0x7893;
    uint16_t result1= 0x8312; // BS data

    result = myI2C->device_read_swap(Convert_Reg);
    printf("raw convert register count is: 0x%x\n", result);

    /************************************
        conversion is:
        (conversion register count)/16 (right shift 4 bits as only 12 bit adc)
        then times the gain, this gives volt count times 1000
        then divide 1000 to set decimal place correctly
    ************************************/
    // check if negative
    if((result & SIGN_MASK) == SIGN_MASK)
        result = -(~(result)+1);
    result = (result>>4)*mv_gain[MV];         /// assumes gain is 2mv/bit, PGA = 4096

    printf("=================== ");
    printf("the voltage (x1) is: %2.3f\n", (float)result/1000);
    float the_result = (float)result/1000 ;

    result = myI2C->device_read_swap(Config_Reg);
    printf("the config register is: x%x\n", result);

    /// this_thread::sleep_for(chrono::milliseconds(200));
    cout<<"~~~ End conversion ~~~"<<endl;
    return the_result;
}
/*************************************************/


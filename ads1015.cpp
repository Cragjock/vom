#include <chrono>		//chrono::milliseconds(1000); need scope chrono
#include <thread>
#include "ads1015.h"
#include "myi2c.h"

using namespace std;

char* mux_type[]=
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

//myADS1015 ADC = {0x48, mv2048, 1, MUX4, DR250SPS};
myADS1015 ADC = {0x48, mv4096, 0, MUX0, DR250SPS};

myADS1015 arrADC[4]=  {
                    {0x48, mv4096, 0, MUX0, DR250SPS},
                    {0x48, mv2048, 1, MUX1, DR250SPS},
                    {0x48, mv1024, 1, MUX4, DR1600SPS},
                    {0x48, 0, 0, 0, 0},
                };


//myADS1015 ADC = {0x48, mv2048, 1, MUX4, DR250SPS};

const char * i2cdev[2] = {"/dev/ic2-0","/dev/i2c-1"};
static int slave_address = 0x48;
static uint16_t default_config_reg = 0x0583;
static uint16_t init_config_reg = 0x4223;

/*************************************************/
 ads1015::ads1015()
 {
    myI2C= unique_ptr<I2CBus>(new I2CBus(1,0x48));
    //myI2C = new I2CBus(1,0x48);

    ADS1015_Init();
 }

 /*************************************************/
 ads1015::ads1015(uint8_t bus, uint8_t address)  //I2CBus(unsigned int bus, unsigned int address);
 {
    myI2C= unique_ptr<I2CBus>(new I2CBus(1,0x48));
    //myI2C = new I2CBus(1,0x48);

    ADS1015_Init();

 }

/*************************************************/
 ads1015::~ads1015()
 {


 }

/*************************************************/
uint8_t ads1015::read_config_reg()
{
    uint8_t result = 67;   //BS number to start
    //UINT resultswap = 17;
    uint16_t resultswap = 17;

    //result=i2c_smbus_read_word_data(file, Config_Reg);
    //result=i2c_smbus_read_word_data(this->file, Config_Reg);
    result = myI2C->myI2C_read_word_data(Config_Reg);
    printf("read config register: %x \n", result);

    //resultswap=myI2C_read_swap(file, Config_Reg);
    resultswap = myI2C->device_read_swap(Config_Reg);
    printf("read swapped config register: %x \n", resultswap);

    myADS1015 current_CR;
    current_CR.data_rate = (result & 0x0e0) >> CR_DR0;
    current_CR.Mux = (result & 0x7000) >> CR_MUX0;    current_CR.PGA = (result &0x0E00 ) >> CR_PGA0;

    arrADC[3].data_rate = (resultswap & 0x0e0) >> CR_DR0;
    arrADC[3].Mux = (resultswap & 0x7000) >> CR_MUX0;
    arrADC[3].PGA = (resultswap &0x0E00 ) >> CR_PGA0;

    printf("current config reg is: %x\ndata rate is: %x\nmux is: %x\nPGA is: %x\n",
                    resultswap,
                    arrADC[3].data_rate,
                    arrADC[3].Mux,
                    arrADC[3].PGA);

    printf("Mux number is: %s \n", mux_type[arrADC[3].Mux]);
    printf("PGA setting is: %d \n", fullscale[arrADC[3].PGA]);
    printf("Data rate is: %d \n", data_rates[arrADC[3].data_rate]);

    return resultswap;
}

/**************************************
    Initialize bus and ADC
**************************************/
int ads1015::ADS1015_Init()
{
    init_config_reg = mux_single_1 | PGA_4096 | DR_250sps | MODE_CONTINUOUS | COMP_QUE_DISABLE; // best is 4096 ?
    /// should be 0x4223
    myI2C->device_write_swap(Config_Reg,init_config_reg);
    ///myI2C_write_swap(file, Config_Reg, init_config_reg);
    //ADS1015_op_init();

    return 1;
}

/*************************************************/
int ads1015::ADS1015_op_init()   /// maybe not needed
{
    char buf[10];
    uint8_t result = 0x1234; // BS number
    UINT res = 0x4823;
    uint16_t result1 = 0x5678;

    //result=i2c_smbus_read_word_data(file, Config_Reg);
    result = myI2C->device_read_swap(Config_Reg);
    printf("the op intit config register is x%x: \n", result);

    result1 = (result & 0x1000); // check if still converting or not
    if((result & 0x1000) == 0x1000)             // if (x & MASK) = MASK, ok
        printf("Not doing a conversion\n");
    else
        printf("conversion in process\n");

    result1 = ((result & 0x7000)>> 12);
    printf("Mux number is :%s \n", mux_type[result1]);       // which MUX type

    result1 = ((result >>5) & 0x0007);
    printf("Data rate is :%d \n", data_rates[result1]);        //data rate
    result1 = ((result >>9) & 0x0007);
    printf("PGA setting is :%d \n\n\n\n", fullscale[result1]);

	UINT config = res;
	config &= 0x001f;                   // strip for COMP items
	config |= (0 << 15) | (0 << 8);     // set for continuous
	config |= (ADC.Mux & 0x0007) << 12;
	config |= (ADC.PGA & 0x0007) << 9;
	config |= (ADC.data_rate & 0x0007) << 5;

    result1 = ((config & 0x7000)>> 12);
    printf("Mux number is :%s \n", mux_type[result1]);       // which MUX type

    result1 = (config >>5)& 0x0007;
    printf("Data rate is: %d SPS\n", data_rates[result1]);        //data rate

    result1 = (config >>9) & 0x0007;
    printf("PGA setting is: %d mv\n", fullscale[result1]);

    result1 = (config >>8)& 0x0001;
    printf("Current mode is: %s\n", c_mode[result1]);

    //=== get the byte order correct ====
    buf[2]=HBYTE(config);
    buf[3]=LBYTE(config);
    result=(buf[3]<<8) | buf[2];    //x = lobyte << 8 | hibyte;
    printf("byte order correct config reg: 0x%x\n", result);

    ///result=i2c_smbus_write_word_data(this->file, Config_Reg, result);
    result= myI2C->myI2C_write_word_data(Config_Reg, result);


    //result=myI2C_write_swap(file, Config_Reg, result);

    //result=i2c_smbus_write_word_data(file, Config_Reg,0x4302);
    //result=i2c_smbus_write_word_data(file, Config_Reg,config);

    //result=i2c_smbus_read_word_data(file, Config_Reg);
    result=read_config_reg();
    //result = myI2C_read_swap(file, Config_Reg);

    //uint16_t sdc = bswap_16(result); // grt the right order to display
    printf("the op intit config register is x%x: \n", result);

    return 0;
}

/*************************************************/
/**< KEEP THIS ************** */
float ads1015::read_conversion()
{
//do
//{
    //SINT result = 23; // BS number
    int16_t result = 0x1234; // due to __s32 i2c_smbus_read_word_data(int file, __u8 command)
    int16_t result_sw = 0x7893;
    uint16_t result1= 0x8312; // BS data

    //result = i2c_smbus_read_word_data(file, Convert_Reg);       // read the data
    //result = myI2C_read_swap(file, Convert_Reg); // read the data
    //result = myI2C_read_swap(file, Convert_Reg); // read the data
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

    result = (result>>4)*2;         /// assumes gain is 2mv/bit, maybe times PGA value?

    printf("===================");
    printf("the voltage (x1) is: %2.3f\n", (float)result/1000);
    float the_result = (float)result/1000 ;

    ///float PCAcount = ((float)result/1000*80)+320;
    ///printf("Test equation for servo count: %2.3i\n", (int)PCAcount);

    //result=i2c_smbus_read_word_data(file, Config_Reg);
    //result = myI2C_read_swap(file, Config_Reg);
    result = myI2C->device_read_swap(Config_Reg);
    printf("the config register is: x%x\n", result);

    //sleep(2);
    this_thread::sleep_for(chrono::milliseconds(200));

//}
//while(1);
    return the_result;
}

/*************************************************/


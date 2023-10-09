# SaveSignaltoFNIRSIGenerator
Just Saves a signal into internal FNIRSI generator DPOX180H, as example PWM
It will write the signal directly into connected scope DPOX180H. Use on your own risk. No responsibility for any damage.
Usage:
Write your own signal into internal FNIRSI generator DPOX180H by modifying a function
GenerateSignal
which has upto 300 samples that you can assign values.
Please note that FNIRSI clamps top quater of a signal and inverst it, so to make a normal signal, do some math that multiplies signal to 0.75

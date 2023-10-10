# SaveSignaltoFNIRSIGenerator
Just Saves a signal into internal FNIRSI generator DPOX180H, as example PWM.

It will write the signal directly into connected scope DPOX180H. Use on your own risk. No responsibility for any damage.

Usage:
1. Modify the code inside GenerateSignal that has a buffer of 300 samples where you can assign values. Please note that FNIRSI clamps top quater of a signal and inverts it, so to make a normal signal, do some math that multiplies signal to 0.75.
2. Connect scope into PC as drive D.
3. Compile in vs and Run. It will create a signal into FNIRSI DPOX180H generator.


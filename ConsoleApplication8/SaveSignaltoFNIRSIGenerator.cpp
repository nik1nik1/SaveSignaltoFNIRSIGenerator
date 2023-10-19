#include <iostream>
#include <cstdio>
#include <cstring>
#include <cmath>
#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>
#include <vector>
#include <Windows.h>

struct Frame {
    short n;
    short l;
    unsigned char w[300];
};

void GenerateSignalPWM(struct Frame& F)
{
    // PWM
    const int T = 300; // points
    const int RAMPS_RES = 20; // triangle points, eg: 10, 12, 20

    for (int i = 0; i < T; i++)
    {
        double s = (RAMPS_RES - 0.5) * ((sin((double)i / (double)T * 2.0 * M_PI) + 1.0) / 2.0) / (double)RAMPS_RES; //sine
        double r = (double(i % RAMPS_RES) / (double)RAMPS_RES); //ramp 0.0 to (RAMPS_RES-1)/RAMPS_RES
        r = 2.0 * (r < 0.5 ? r : 1.0 - r); // turn rump into triangle

        unsigned char o = s > r ? UCHAR_MAX : 0; // PWM
        
        //unsigned char o = round(r * (double)UCHAR_MAX*0.75); // Triangle

        //printf("%d %f %f\n",  o, s, r);
        F.w[i] = o;
    }
    F.l = _byteswap_ushort(T+1); // Big Endian. Scope needs +1, otherwise it cuts last sample.
}

void GenerateSignalPWM4Inverter(struct Frame& F)
{
    // PWM like Pure sine inverter would output before filter
    const int T = 300; // points
    const int RAMPS_RES = 20; // triangle points, eg: 10, 12, 20

    for (int i = 0; i < T; i++)
    {




        double s = (RAMPS_RES - 0.5) * (sin((double)i / (double)T * 2.0 * M_PI)) / (double)RAMPS_RES; //sine
        double r = (double(i % RAMPS_RES) / (double)RAMPS_RES); //ramp 0.0 to (RAMPS_RES-1)/RAMPS_RES
        r = 2.0 * (r < 0.5 ? r : 1.0 - r); // turn rump into triangle

        unsigned char o = s > r ? UCHAR_MAX : (-s > r? 0 : 96); // where 96 is round(0.75*UCHAR_MAX/2.0)

        //unsigned char o = round(r * (double)UCHAR_MAX*0.75); // Triangle

        //printf("%d %f %f\n",  o, s, r);
        F.w[i] = o;
    }
    F.l = _byteswap_ushort(T + 1); // Big Endian. Scope needs +1, otherwise it cuts last sample.
}

void GenerateSignalHiLo(struct Frame& F)
{
    // Interleaved Max and Min samples. Let get freq up to 90MHz at (600kHz -1) * 300 points
    const int T = 300; // points
    for (int i = 0; i < T; i++)
    {
        F.w[i] = i%2? UCHAR_MAX : 0;
    }
    F.l = _byteswap_ushort(T+1); // Big Endian. Scope needs +1, otherwise it cuts last sample.
}

void GenerateSignalDirac(struct Frame& F)
{
    // Single pulse in the middle
    const int T = 300; // points
    // Just one single point in the middle
    F.w[T/2] = UCHAR_MAX;
    F.l = _byteswap_ushort(T+1); // Big Endian. Scope needs +1, otherwise it cuts last sample.
}

void GenerateSignalSine(struct Frame& F)
{
    // sine
    const int T = 300; // points

    for (int i = 0; i < T; i++)
    {
        double s = 0.75*((sin((double)i / (double)T * 2.0 * M_PI) + 1.0) / 2.0); //sine 0 to 0.75 as FNIRSI clamps signal over that range

        F.w[i] = (unsigned char) round((double)UCHAR_MAX * s);
    }
    F.l = _byteswap_ushort(T+1); // Big Endian. Scope needs +1, otherwise it cuts last sample.
}

void GenerateSignalAM(struct Frame& F)
{
    // AM
    const int T = 300; // total points, equal to modulated sine points.
    const int t = 2;   // carrier sine points. Set 2 to get max carrier freq. Set 15 to see low carier freq.

    const double Ac = 0.51; // Carrier Amplitude
    const double Am = 0.49; // Modulated Amplitude
    // where Am+Ac <= 1.0, otherwise signal will clamp

    for (int i = 0; i < T; i++)
    {
        double s = (Ac + Am * cos((double)i / (double)T * M_PI*2.0)) * cos((double)i / (double)t * M_PI*2.0) ; // AM -1.0 to 1.0
        s = 0.75 * (1.0 + s) / 2.0; // AM 0.0 to 0.75

        F.w[i] = (unsigned char) round((double)UCHAR_MAX * s);
    }
    F.l = _byteswap_ushort(T+1); // Big Endian. Scope needs +1, otherwise it cuts last sample.
}

void GenerateSignalFM(struct Frame& F)
{
    // FM
    const int T = 300; // total points, equal to modulated sine points.
    const int t = 12;  // carrier sine points.

    const double Ac = 1.0;    // Carrier Amplitude
    const double Bdev = 20.0; // Modulated deviation. Too Low values makes modulation barely visible on scope.

    for (int i = 0; i < T; i++)
    {
        double s = Ac * sin((double)i / (double)t * M_PI * 2.0  + Bdev * sin((double)i / (double)T * M_PI * 2.0)) ; // FM -1.0 to 1.0
        s = 0.75 * (1.0 + s) / 2.0; // AM 0.0 to 0.75

        F.w[i] = (unsigned char)round((double)UCHAR_MAX * s);
    }
    F.l = _byteswap_ushort(T + 1); // Big Endian. Scope needs +1, otherwise it cuts last sample.
}

void GenerateSignal(struct Frame& F)
{
    // Call only one func here!
    GenerateSignalAM(F);
}

int main()
{
    std::ifstream file("D:\\Captures\\data.cap", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (size == -1) return 0; // Exit if no device connected

    const size_t nFrames = size / sizeof(struct Frame);
    std::vector<struct Frame> buffer(nFrames);
    if (file.read((char *)buffer.data(), size))
    {
        for (size_t j = 0; j< nFrames; j++)
        {
            if (buffer[j].l == 0)
            {
                buffer[j].n = _byteswap_ushort((unsigned short)j+1);

                GenerateSignal(buffer[j]);

                std::ofstream data_cap("D:\\Captures\\data.cap", std::ios::binary);
                data_cap.write((char*)buffer.data(), size);

                // Indexes

                std::ifstream num_table_sysR("D:\\Captures\\num_table.sys", std::ios::binary | std::ios::ate);
                std::streamsize num_table_sysRSize = num_table_sysR.tellg();
                num_table_sysR.seekg(0, std::ios::beg);

                std::vector<unsigned short> num_table_sysRbuffer(num_table_sysRSize/2);
                if (num_table_sysR.read((char*)num_table_sysRbuffer.data(), num_table_sysRSize))
                {
                    unsigned short nFrame = j + 1;
                    for (int k = 2; k < num_table_sysRSize/2; k++)
                    {
                        if (num_table_sysRbuffer[k] <= j + 1)
                        {
                            unsigned short tnFrame = num_table_sysRbuffer[k];
                            num_table_sysRbuffer[k] = nFrame;
                            if (tnFrame == 0) {
                                num_table_sysRbuffer[0]++; // Update the count of Captures
                                //num_table_sysRbuffer[1] = j + 1; // Load Capture into generator
                                break;
                            }
                            else nFrame = tnFrame;
                        }
                    }
                    
                    int attr = GetFileAttributes(L"D:\\Captures\\num_table.sys");
                    SetFileAttributes(L"D:\\Captures\\num_table.sys", 0); // Write can not handle system hidden archive file, so remove attributes first
                    
                    std::ofstream num_table_sysW("D:\\Captures\\num_table.sys", std::ios::binary);
                    num_table_sysW.write((char*)num_table_sysRbuffer.data(), num_table_sysRSize);
                    SetFileAttributes(L"D:\\Captures\\num_table.sys", attr);
                }

                break;
            }
        }
    }
}

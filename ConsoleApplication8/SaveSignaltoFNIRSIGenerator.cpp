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
    const int RAMPS_RES = 15; // triangle points

    for (int i = 0; i < T; i++)
    {
        double s = (RAMPS_RES - 1) * ((sin((double)i / (double)T * 2.0 * M_PI) + 1.0) / 2.0) / (double)RAMPS_RES + 0.5 / (double)RAMPS_RES; //sine
        double r = (double(i % RAMPS_RES) / (double)RAMPS_RES); //ramp
        r = 2.0 * (r < 0.5 ? r : 1.0 - r); // turn rump into triangle

        unsigned char o = s >= r ? UCHAR_MAX : 0; // PWM

        //printf("%d %f %f\n",  o, s, r);
        F.w[i] = o;
    }
    F.l = _byteswap_ushort(T); // Length is Big Endian
}

void GenerateSignalHiLo(struct Frame& F)
{
    // Interleaved Max and Min samples
    const int T = 300; // points
    for (int i = 0; i < T; i++)
    {
        F.w[i] = i%2* UCHAR_MAX;
    }
    F.l = _byteswap_ushort(T); // Big Endian
}

void GenerateSignalDirac(struct Frame& F)
{
    const int T = 300; // points
    // Just one single point in the middle
    F.w[T/2] = UCHAR_MAX;
    F.l = _byteswap_ushort(T); // Big Endian
}

void GenerateSignal(struct Frame& F)
{
    GenerateSignalPWM(F);
}

int main()
{
    std::ifstream file("D:\\Captures\\data.cap", std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<struct Frame> buffer(size/sizeof(struct Frame));
    if (file.read((char *)buffer.data(), size))
    {
        for (int j = 0; ; j++)
        {
            if (buffer[j].l == 0)
            {
                buffer[j].n = _byteswap_ushort(j+1);

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
                    for (int k = 2; ; k++)
                    {
                        if (num_table_sysRbuffer[k] == 0)
                        {
                            num_table_sysRbuffer[0] = k - 1;
                            for (int l = 0; l < k; l++) num_table_sysRbuffer[l+2] = k-1-l;
                            break;
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

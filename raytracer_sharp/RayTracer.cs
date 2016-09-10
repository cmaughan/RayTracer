using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Tracer
{
    class RayTracer
    {
        public static int ImageWidth = 512;
        public static int ImageHeight = 512;

        public unsafe void Run(byte* pData, int stride)
        {
            for (int y = 0; y < ImageHeight; y++)
            {
                for (int x = 0; x < ImageWidth; x++)
                {
                    var pPixel = (UInt32*)(pData + (y * stride) + (x * 4));
                    *pPixel = 0xFFF00FFF;
                }
            }
        }
    }
}

using System;
using Windows.ApplicationModel.Core;

namespace FrameworklessUWP
{
    public sealed class Program
    {
        static void Main(string[] args)
        {
            CoreApplication.Run(new App());
        }
    }
}

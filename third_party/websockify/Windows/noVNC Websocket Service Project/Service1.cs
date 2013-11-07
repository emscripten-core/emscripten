using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.IO;

namespace MELT_Command_Websocket
{
    public partial class Service1 : ServiceBase
    {
        Process websockify;
        public Service1()
        {
            InitializeComponent();
        }

        protected override void OnStart(string[] args)
        {

            string configpath = AppDomain.CurrentDomain.BaseDirectory + "\\noVNCConfig.ini";
            string sockifypath = AppDomain.CurrentDomain.BaseDirectory + "\\websockify.exe";
            //Load commandline arguements from config file.
            StreamReader streamReader = new StreamReader(configpath);
            string arguements = streamReader.ReadLine();
            streamReader.Close();
           
            //Start websockify.
            websockify = System.Diagnostics.Process.Start(sockifypath, arguements);
        }

        protected override void OnStop()
        {
            //Service stopped. Close websockify.
            websockify.Kill();
        }
    }
}

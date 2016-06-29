//#######################################################################################################
//#################################### Plugin 033: Relay with button/switch #############################
//#######################################################################################################

#define PLUGIN_033
#define PLUGIN_ID_033         33
#define PLUGIN_NAME_033       "Relay with Button/Switch"
#define PLUGIN_VALUENAME1_033 "Relaybutton"

boolean Plugin_033(byte function, struct EventStruct *event, String& string)
{
  /* The first GPIO is used for connecting the Relay
  The second GPIO is used for connecting the Button or Switch*/

  boolean success = false;
  static byte switchstate[TASKS_MAX];

  switch (function)
  {
    case PLUGIN_DEVICE_ADD: // Declaration of possible settings
      {
        Device[++deviceCount].Number = PLUGIN_ID_033; //Specify the plugin ID
        Device[deviceCount].Type = DEVICE_TYPE_DUAL; //Can be Single or Dual, means the number of GPIO used
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH; //Sensor type in Domoticz or other domotic system
        Device[deviceCount].Ports = 0; //I don't know
        Device[deviceCount].PullUpOption = true; // Option for PullUP
        Device[deviceCount].InverseLogicOption = false; // Option to inverse the working logic
        Device[deviceCount].FormulaOption = false; // Option to let user specify a formula
        Device[deviceCount].ValueCount = 1; // I don't know
        Device[deviceCount].SendDataOption = true; // Option to let user choose if send data or work locally
        Device[deviceCount].TimerOption = false; // Option to let user set a delay
        Device[deviceCount].GlobalSyncOption = true; // I don't know
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        String buttonOptions[3];
        buttonOptions[0] = F("Switch");
        buttonOptions[1] = F("Button Active Low");
        buttonOptions[2] = F("Button Active High");
        int buttonOptionValues[3];
        buttonOptionValues[0] = 0;
        buttonOptionValues[1] = 1;
        buttonOptionValues[2] = 2;
        string += F("<TR><TD>Switch/Button Type:<TD><select name='plugin_033_button'>");
        for (byte x = 0; x < 3; x++)
        {
          string += F("<option value='");
          string += buttonOptionValues[x];
          string += "'";
          if (choice == buttonOptionValues[x])
            string += F(" selected");
          string += ">";
          string += buttonOptions[x];
          string += F("</option>");
        }
        string += F("</select>");

        string += F("<TR><TD>Send Boot state:<TD>");
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1])
          string += F("<input type=checkbox name=plugin_033_boot checked>");
        else
          string += F("<input type=checkbox name=plugin_033_boot>");

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String plugin0 = WebServer.arg("plugin_033_button");
        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = plugin0.toInt();

        String plugin1 = WebServer.arg("plugin_033_boot");
        Settings.TaskDevicePluginConfig[event->TaskIndex][1] = (plugin1 == "on");

        success = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME: // Default function to get plugin name
      {
        string = F(PLUGIN_NAME_033);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES: // Default function to get plugin value names
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_033));
        break;
      }

    case PLUGIN_INIT: // Function runned at boot time or during configuration to inizialize plugin
      {
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], OUTPUT); // Setting the relay pin to output
        setPinState(PLUGIN_ID_033, Settings.TaskDevicePin1[event->TaskIndex], PIN_MODE_OUTPUT, 1);

        if (Settings.TaskDevicePin1PullUp[event->TaskIndex]) // Setting the button/switch pin to input
          pinMode(Settings.TaskDevicePin2[event->TaskIndex], INPUT_PULLUP);
        else
          pinMode(Settings.TaskDevicePin2[event->TaskIndex], INPUT);

        setPinState(PLUGIN_ID_033, Settings.TaskDevicePin2[event->TaskIndex], PIN_MODE_INPUT, 0);

        switchstate[event->TaskIndex] = digitalRead(Settings.TaskDevicePin1[event->TaskIndex]);
        
        // if boot state must be send, inverse default state
        if (Settings.TaskDevicePluginConfig[event->TaskIndex][1])
        {
          switchstate[event->TaskIndex] = !switchstate[event->TaskIndex];
        }
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND: //Is this function runned 10 times per second?
      {
        byte state = digitalRead(Settings.TaskDevicePin2[event->TaskIndex]); // Read the button GPIO

        if (state != switchstate[event->TaskIndex]) //Check if different from last read otherwise do not do anything
        {
          switchstate[event->TaskIndex] = state; //Save new button state
          if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 0) // Switch
          {
            triggerRelay(event);
          } 
          else
          {
            if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 1) // Button Active Low
            {
              //TODO Button Low Logic
              if(state == 0)
              {
                triggerRelay(event);
              }
            }
            else
            {
              if (Settings.TaskDevicePluginConfig[event->TaskIndex][0] == 2) // Button Active High
              {
                if(state == 1)
                {
                  triggerRelay(event);
                }
              }
            }
          }
        }
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        // We do not actually read the pin state as this is already done 10x/second
        // Instead we just send the last known state stored in Uservar
        String log = F("SW   : State ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }
  }
  return success;
}

/*********************************************************************/
void triggerRelay(struct EventStruct *event)
{
  byte stateSwitch = digitalRead(Settings.TaskDevicePin1[event->TaskIndex]); // Read the Relay GPIO
  digitalWrite(Settings.TaskDevicePin1[event->TaskIndex], !stateSwitch); // Inverse the Relay
  event->sensorType = SENSOR_TYPE_SWITCH;
  UserVar[event->BaseVarIndex] = stateSwitch;

  String log = F("SW   : State ");
  log += !stateSwitch;
  addLog(LOG_LEVEL_INFO, log);
  sendData(event);
}


#include <LeweiTcpClient.h>

/*
UserFunction::UserFunction(void (*callfuct)(),const char *uFunctionName)
{
	userFunctionAddr0=callfuct;
	userFunctionAddr1=NULL;
	userFunctionAddr2=NULL;
	//userFunctionAddr3=NULL;
	//userFunctionAddr4=NULL;
	//userFunctionAddr5=NULL;
	userFunctionName=uFunctionName;
	next = NULL;
}
UserFunction::UserFunction(void (*callfuct)(char*),const char *uFunctionName)
{
	userFunctionAddr0=NULL;
	userFunctionAddr1=callfuct;
	userFunctionAddr2=NULL;
	//userFunctionAddr3=NULL;
	//userFunctionAddr4=NULL;
	//userFunctionAddr5=NULL;
	userFunctionName=uFunctionName;
	next = NULL;
}
UserFunction::UserFunction(void (*callfuct)(char*,char*),const char *uFunctionName)
{
	userFunctionAddr0=NULL;
	userFunctionAddr1=NULL;
	userFunctionAddr2=callfuct;
	//userFunctionAddr3=NULL;
	//userFunctionAddr4=NULL;
	//userFunctionAddr5=NULL;
	userFunctionName=uFunctionName;
	next = NULL;
}

UserFunction::UserFunction(void (*callfuct)(char*,char*,char*),const char *uFunctionName)
{
	userFunctionAddr0=NULL;
	userFunctionAddr1=NULL;
	userFunctionAddr2=NULL;
	userFunctionAddr3=callfuct;
	userFunctionAddr4=NULL;
	userFunctionAddr5=NULL;
	userFunctionName=uFunctionName;
	next = NULL;
}
UserFunction::UserFunction(void (*callfuct)(char*,char*,char*,char*),const char *uFunctionName)
{
	userFunctionAddr0=NULL;
	userFunctionAddr1=NULL;
	userFunctionAddr2=NULL;
	userFunctionAddr3=NULL;
	userFunctionAddr4=callfuct;
	userFunctionAddr5=NULL;
	userFunctionName=uFunctionName;
	next = NULL;
}
UserFunction::UserFunction(void (*callfuct)(char*,char*,char*,char*,char*),const char *uFunctionName)
{
	userFunctionAddr0=NULL;
	userFunctionAddr1=NULL;
	userFunctionAddr2=NULL;
	userFunctionAddr3=NULL;
	userFunctionAddr4=NULL;
	userFunctionAddr5=callfuct;
	userFunctionName=uFunctionName;
	next = NULL;
}
*/




UserSwitch::UserSwitch(void (*uSwitchFunctionAddr)(char*),const char *uSwitchId,int uSwitchState)
{
	//userSwitchOnFunctionAddr=uSwitchOnFunctionAddr;
	//userSwitchOffFunctionAddr=uSwitchOffFunctionAddr;
	userSwitchFunctionAddr = uSwitchFunctionAddr;
	userSwitchId=uSwitchId;
	userSwitchState=uSwitchState;
	next = NULL;
}

LeweiTcpClient::LeweiTcpClient(const char *userKey,const char *gatewayNo):
	server(EthernetServer(80)),
	tcpServer("tcp.lewei50.com")
{
	_userKey = (char*)malloc( strlen(userKey)+1 );
	strcpy( _userKey,userKey );
	
	_gatewayNo = (char*)malloc( strlen(gatewayNo)+1 );
	strcpy( _gatewayNo,gatewayNo );
	
	setupDefaultValue();
	
	//Ethernet.begin(_mac);
	byte _mac[] = {0x74, 0x69, 0x69, 0x2D, 0x30, 0x31};
	if (Ethernet.begin(_mac) == 0) {
    Serial.println("fail");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }
	Serial.println(Ethernet.localIP());
	//Ethernet.begin(mac, ip);
	delay(1000);
	String clientStr="";
	keepOnline();
}
/*
LeweiTcpClient::LeweiTcpClient(const char *userKey,const char *gatewayNo,byte mac[]):server(EthernetServer(80))
{
	_userKey = userKey;
	_gatewayNo = gatewayNo;
	setupDefaultValue();
	
	Ethernet.begin(mac);
	Serial.println(Ethernet.localIP());
	delay(1000);
	
	String clientStr="";
	keepOnline();
}
*/

LeweiTcpClient::LeweiTcpClient( const char *userKey,const char *gatewayNo,byte mac[],IPAddress ip,IPAddress dns,IPAddress gw,IPAddress subnet):
	server(EthernetServer(80)),
	tcpServer("tcp.lewei50.com")
{
	_userKey = (char*)malloc( strlen(userKey)+1 );
	strcpy( _userKey,userKey );
	
	_gatewayNo = (char*)malloc( strlen(gatewayNo)+1 );
	strcpy( _gatewayNo,gatewayNo );
	
	setupDefaultValue();
	Ethernet.begin(mac,ip,dns,gw,subnet);
	Serial.println(Ethernet.localIP());
	delay(1000);
	String clientStr="";
	keepOnline();
}

void LeweiTcpClient::setupDefaultValue()
{
	//checkFreeMem();
	//head = NULL;
	switchHead = NULL;
	
	byte _mac[] = {0x74, 0x69, 0x69, 0x2D, 0x30, 0x31};
	_postInterval = 30000;//server setting is 60000
	_starttime = millis();
	
	int len=strlen(_gatewayNo)+32+51;//api-key length:32
	
	aliveString=(char *)malloc(len);	
	snprintf(aliveString, len, "{\"method\":\"update\",\"gatewayNo\":\"%s\",\"userkey\":\"%s\"}&^!", _gatewayNo, _userKey);
	

	setRevCtrlMsg("false","NotBind");


	setRevCtrlData("");
	_bEasyMode = false;
	startCommPost = 75;
	currentPos = 0;

}

void LeweiTcpClient::easySetupMode(boolean bEasyMode)
{
	_bEasyMode = bEasyMode;
	if(_bEasyMode)
	{
		readRom();
		int len=strlen(_gatewayNo)+32+51;//api-key length:32
		aliveString=(char *)realloc(aliveString,len);	
		snprintf(aliveString, len, "{\"method\":\"update\",\"gatewayNo\":\"%s\",\"userkey\":\"%s\"}&^!", _gatewayNo, _userKey);
		
		server.begin();
	  //Serial.print("srv:");
	  //Serial.println(Ethernet.localIP());
	}
}

void LeweiTcpClient::writeRom(String value)
{
  for(int i =0;i<52;i++)
  {
     EEPROM.write(i, value.charAt(i));
  }
  softwareReset();
}

void LeweiTcpClient::softwareReset()
{
   asm volatile ("  jmp 0");
}



void LeweiTcpClient::readRom()
{
  byte value;
  String tmp = "";
  for(int address=0;address<52;address++)
  {
    value = EEPROM.read(address);
    
    if(value!=0x00)
    {
   		tmp +=char(value);
   	}
    //Serial.print(address);
    //Serial.println(char(value));
    
    if(address==31)
    {
    	if(tmp.length()==32)
    	{
	    	char * tmpc = strToChar(tmp);
	    	_userKey = tmpc;
	    	//Serial.print("ky:");
	    	//Serial.println(_userKey);
	    	//free(tmpc);
	    	//tmpc = NULL;
	    }
    	tmp = "";
    }
    else if(address==51)
    {
    	if(tmp.length()>0)
    	{
	    	char * tmpc = strToChar(tmp);
	    	_gatewayNo = tmpc;
	    }
    	tmp = "";
    }
  }
}


void LeweiTcpClient::listenServer()
{
  EthernetClient clientWeb = server.available();
  if (clientWeb) {
    //Serial.println("new clientWeb");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String clientStr = "";
    while (clientWeb.connected()) {
      if (clientWeb.available()) {
        char c = clientWeb.read();
        clientStr += c;
        //Serial.write(c);
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          clientWeb.println("<html><body><form method='get'>");
          clientWeb.print("KEY<input type='text' name='a' value='");
          clientWeb.print(_userKey);
          clientWeb.print("'>GW<input type='text' name='g' value='");
          clientWeb.print(_gatewayNo);
          clientWeb.println("'><input type='submit'>");
          clientWeb.println("</form></body></html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
          if(clientStr.indexOf(" /?a=")>0)
          {
          	Serial.print(clientStr);
          	String userInfoStr = clientStr.substring(clientStr.indexOf(" /?a=")+5,clientStr.indexOf("&g="));
          	userInfoStr += clientStr.substring(clientStr.indexOf("&g=")+3,clientStr.indexOf(" HTTP/1.1"));
            if(userInfoStr.length()>33)writeRom(userInfoStr);
            Serial.println(userInfoStr);
          }
          //Serial.println(clientStr);
          clientStr = "";
          //checkFreeMem();
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    clientWeb.stop();
    clientWeb= NULL;
    //Serial.println("clientWeb disonnected");
  }
  
}

char* LeweiTcpClient::strToChar(String str)
{
	char* c = (char*)malloc(str.length()+1);
	if(!c)
	{
		//Serial.print("strToChar::");
		//Serial.println(str);
		Serial.println("malloc:F");
		return NULL;
	}
	str.toCharArray(c,str.length()+1);
	return c;
}

void LeweiTcpClient::sendOnlineCommand()
{
		_clientRevCtrl.print(aliveString);
		//Serial.println(aliveString);
}

void LeweiTcpClient::keepOnline()
{
	getResponse();
	
	if(_bEasyMode)
	{
		listenServer();
	}
		if (_clientRevCtrl.connected()) 
		{
			if ((millis()-_starttime) > _postInterval)
			{
				_starttime = millis();
					sendOnlineCommand();
			}
		}
		else
		{
			connentTcpServer();
		}
}

void LeweiTcpClient::sendUserSwitchState()
{
	UserSwitchNode *currentSwitch = switchHead;                    
	boolean bFirstNode = true;
	//{\"id\":\"m\",\"name\":\"m\",\"value\":\"0\",\"status\":\"ok\"}
	String stateStr = "[";
	while(currentSwitch != NULL)
	{
		if(!bFirstNode)stateStr+=",";
		stateStr+="{\"id\":\"";
		stateStr+=currentSwitch->userSwitchId;
		stateStr+="\",\"name\":\"";
		stateStr+=currentSwitch->userSwitchId;
		stateStr+="\",\"value\":\"";
		stateStr+=String(currentSwitch->userSwitchState);
		stateStr+="\",\"status\":\"ok\"}";
		//Serial.println(functionName);
		//Serial.println(currentSwitch->userSwitchId);
		//Serial.println(currentSwitch->userSwitchState);     
		bFirstNode = false;
		currentSwitch = currentSwitch->next;
	}
	stateStr+="]";
	//Serial.println(stateStr);
	
	if(!bFirstNode)
	{
		setRevCtrlMsg("true","ok");
		char * msg = strToChar(stateStr);
		setRevCtrlData(msg);
		//msg = NULL;
	}
	stateStr = "";
	//stateStr = NULL;
	if(strlen(_revCtrlData)>0)
	{
		int len=strlen(_revCtrlResult)+strlen(_revCtrlMsg)+strlen(_revCtrlData)+71;
		commandString=(char *)malloc(len);	
		snprintf(commandString, len, "{\"method\":\"response\",\"result\":{\"successful\":%s,\"message\":\"%s\",\"data\":%s}}&^!", _revCtrlResult, _revCtrlMsg,_revCtrlData);
		
		_clientRevCtrl.print(commandString);
		free(commandString);
		commandString = NULL;
		free(_revCtrlData);
		_revCtrlData = NULL;
		setRevCtrlMsg("true","ok");
		setRevCtrlData("");
		_clientStr = "";
	}
}

void LeweiTcpClient::updateUserSwitchState(char* switchId,char* switchStat)
{
	UserSwitchNode *currentSwitch = switchHead;                    
	boolean bFirstNode = true;
	//{\"id\":\"m\",\"name\":\"m\",\"value\":\"0\",\"status\":\"ok\"}
	String stateStr = "[";
	while(currentSwitch != NULL)
	{
		if(String(switchId).equals(String(currentSwitch->userSwitchId)))
		{
			if(!bFirstNode)stateStr+=",";
			stateStr+="{\"id\":\"";
			stateStr+=currentSwitch->userSwitchId;
			//stateStr+="\",\"name\":\"";
			//stateStr+=currentSwitch->userSwitchId;
			stateStr+="\",\"value\":\"";
			stateStr+=switchStat;
			stateStr+="\",\"status\":\"ok\"}"; 
			bFirstNode = false;
		}
		currentSwitch = currentSwitch->next;
	}
	stateStr+="]";
	//Serial.println(stateStr);
	
	if(!bFirstNode)
	{
		setRevCtrlMsg("true","ok");
		char * msg = strToChar(stateStr);
		setRevCtrlData(msg);
		msg = NULL;
	}
	
	if(strlen(_revCtrlData)>0)
	{
		int len=strlen(_revCtrlResult)+strlen(_revCtrlMsg)+strlen(_revCtrlData)+71;
		commandString=(char *)malloc(len);	
		snprintf(commandString, len, "{\"method\":\"response\",\"result\":{\"successful\":%s,\"message\":\"%s\",\"data\":%s}}&^!", _revCtrlResult, _revCtrlMsg,_revCtrlData);
		
		_clientRevCtrl.print(commandString);
		free(commandString);
		commandString = NULL;
		free(_revCtrlData);
		_revCtrlData = NULL;
		
		setRevCtrlMsg("true","ok");
		setRevCtrlData("");
		_clientStr = "";
	}
}

void LeweiTcpClient::getResponse()
{
	if (_clientRevCtrl.available())
	{
		currentPos ++;
		char c = _clientRevCtrl.read();
		if(currentPos >startCommPost)
		{
			//Serial.print(c);checkFreeMem();
			_clientStr += c;
		}
	}
	else if(_clientStr.length()>0)
	{
		currentPos = 0;
		//Serial.print("FrmSrv:");
		//Serial.println(_clientStr);
		//if(_clientStr.indexOf("&^!")<0)
		//{
			//Serial.println("no end!");
			//return;
		//}
		//checkFreeMem();
		
		String functionName = getParaValueStr(_clientStr,"f");
			char* p1 = getParaValue(_clientStr,"p1");
			char* p2 = getParaValue(_clientStr,"p2");
			//char* p3 = getParaValue(_clientStr,"p3");
			//char* p4 = getParaValue(_clientStr,"p4");
			//char* p5 = getParaValue(_clientStr,"p5");
			_clientStr = "";
			
		if(!functionName.equals(""))//here comes user defined command
		{
  		//Serial.print("f:");
			//Serial.println(functionName);
			if(functionName.equals("getAllSensors"))//try to return the switch status list to server
			{
				sendUserSwitchState();
				free(p1);free(p2);//free(p3);free(p4);free(p5);
				p1=p2=NULL;
				free(commandString);
				commandString = NULL;
				
				return;
				
			}
			else if(functionName.equals("updateSensor"))
			{
				//Serial.println("updateSensor");
				
				UserSwitchNode *currentSwitch = switchHead;  
				while(currentSwitch != NULL)
				{ 
					if(String(p1).equals(String(currentSwitch->userSwitchId)))
					{
						currentSwitch->userSwitchState = String(p2).toInt();
						/*
						if(String(p2).equals("0"))
						{
								currentSwitch->userSwitchState = false;
								//execute(currentSwitch->userSwitchOnFunctionAddr);
						}
						else
						{
								currentSwitch->userSwitchState = true;
								//execute(currentSwitch->userSwitchOffFunctionAddr);
						}
						*/
						
						execute(currentSwitch->userSwitchFunctionAddr,p2);
						updateUserSwitchState(p1,p2);
						//return;
					}
					currentSwitch = currentSwitch->next;
				}
			}
			//UserFunctionNode * current = head;
			/*
			while(current!=NULL)
			{
//				Serial.println(functionName);
//				Serial.println(current->userFunctionName);
				if(functionName.equals(current->userFunctionName))
				{
		//checkFreeMem();
					//setRevCtrlMsg("true","execute function");
					
					if(p5!=NULL)
					{
						//Serial.println(5);
						execute(current->userFunctionAddr5,p1,p2,p3,p4,p5);
					}
					else 
						if(p4!=NULL)
					{
						//Serial.println(4);
						execute(current->userFunctionAddr4,p1,p2,p3,p4);
					}
					else 
						if(p3!=NULL)
					{
						//Serial.println(3);
						execute(current->userFunctionAddr3,p1,p2,p3);
					}
					else 
						if(p2!=NULL)
					{
						//Serial.println(2);
						execute(current->userFunctionAddr2,p1,p2);
					}
					else 
					if(p1!=NULL)
					{
						//Serial.println(1);
						execute(current->userFunctionAddr1,p1);
					}
					else
					{
						//Serial.println(0);
						execute(current->userFunctionAddr0);
					}
					//execute(current->userFunctionAddr);
					break;
				}
				current = current->next;
			}
			*/
		}
			//free(p1);free(p2);free(p3);free(p4);free(p5);
			//p1=p2=p3=p4=p5=NULL;
			free(p1);free(p2);
			p1=p2=NULL;

			functionName = "";
			if(strlen(_revCtrlData)>0)
			{
				int len=strlen(_revCtrlResult)+strlen(_revCtrlMsg)+strlen(_revCtrlData)+71;
				commandString=(char *)malloc(len);	
				snprintf(commandString, len, "{\"method\":\"response\",\"result\":{\"successful\":%s,\"message\":\"%s\",\"data\":%s}}&^!", _revCtrlResult, _revCtrlMsg,_revCtrlData);
				
			}
			else
			{
				int len=strlen(_revCtrlResult)+strlen(_revCtrlMsg)+63;
				commandString=(char *)malloc(len);	
				snprintf(commandString, len, "{\"method\":\"response\",\"result\":{\"successful\":%s,\"message\":\"%s\"}}&^!", _revCtrlResult, _revCtrlMsg);
			}
			//Serial.println(commandString);
			_clientRevCtrl.print(commandString);
			free(commandString);
			commandString = NULL;
		setRevCtrlMsg("false","NotBind");
		setRevCtrlData("");
		_clientStr = "";
		
	}
}

char* LeweiTcpClient::getParaValue(String &orig,String paraName)
{
		//Serial.print("P:");
		//Serial.println(paraName);
		int functionNameStartPos = orig.indexOf("\""+paraName+"\":\"");
		if(functionNameStartPos<0)return NULL;
		int functionNameEndPos = orig.indexOf("\"",functionNameStartPos+4+paraName.length());
		String functionName = orig.substring(functionNameStartPos+4+paraName.length(),functionNameEndPos);
		
		char* c = strToChar(functionName);
		return c;
}

String LeweiTcpClient::getParaValueStr(String &orig,String paraName)
{
		int functionNameStartPos = orig.indexOf("\""+paraName+"\":\"");
		if(functionNameStartPos<0)return "";
		int functionNameEndPos = orig.indexOf("\"",functionNameStartPos+4+paraName.length());
		String functionName = orig.substring(functionNameStartPos+4+paraName.length(),functionNameEndPos);
		
		return functionName;
}
/*
void LeweiTcpClient::directResponse(String respStr)
{
	if(_clientRevCtrl.connected())
	{
		respStr +="&^!";
		char* c = (char*)malloc(respStr.length()+1);
		respStr.toCharArray(c,respStr.length()+1);
		_clientRevCtrl.print(c);
		Serial.println(c);
		free(c);
		c = NULL;
	}
}
*/
void LeweiTcpClient::connentTcpServer()
{
	Serial.print("Connect");
	
	_clientRevCtrl.stop();
	if (_clientRevCtrl.connect(tcpServer, 9960))
	{
		Serial.println("ed");
		sendOnlineCommand();		
	}
	else 
	{
		_clientRevCtrl.stop();
		// if you didn't get a connection to the server:
		
		Serial.println("Fail");
	}
}

void LeweiTcpClient::setRevCtrlMsg(char* execResult,char* msg)
{
	_revCtrlResult = execResult;
	_revCtrlMsg = msg;
}

void LeweiTcpClient::setRevCtrlData(char* data)
{
	_revCtrlData = data;
}
/*
void LeweiTcpClient::appendSensorValue(String sensorName,String sensorValue)
{
	_sensorValueStr +="{\"Name\":\"";
	_sensorValueStr +=sensorName;
	_sensorValueStr +="\",\"Value\":\"";
	_sensorValueStr +=sensorValue;
	_sensorValueStr +="\"},";
}
void LeweiTcpClient::appendSensorValue(String sensorName,int sensorValue)
{
	appendSensorValue(sensorName,String(sensorValue));
}
void LeweiTcpClient::appendSensorValue(String sensorName,float sensorValue)
{
	char s[10];
	int val_len = snprintf(s, 10,"%d.%02u", (int)sensorValue, (int)(abs(sensorValue)*100+0.5) % 100);
	appendSensorValue(sensorName,(String)s);
}

void LeweiTcpClient::appendSensorValue(String sensorName,double sensorValue)
{
	char s[10];
	int val_len = snprintf(s, 10,"%d.%02u", (int)sensorValue, (int)(abs(sensorValue)*100+0.5) % 100);
	appendSensorValue(sensorName,(String)s);
}
void LeweiTcpClient::appendSensorValue(String sensorName,long sensorValue)
{
	appendSensorValue(sensorName,(String)sensorValue);
}
*/

/*
void LeweiTcpClient::sendSensorValue(String sensorName,String sensorValue)
{
	if (_clientRevCtrl.connected())
	{
		//checkFreeMem();
		String connStr = "{\"method\": \"upload\", \"data\":[";
		connStr+=_sensorValueStr;
		connStr+="{\"Name\":\"";
		connStr+=sensorName;
		connStr+="\",\"Value\":\"";
		connStr+=sensorValue;
		connStr+="\"}]}&^!";
		char* c = (char*)malloc(connStr.length()+1);
		if(c)
		{
			connStr.toCharArray(c,connStr.length()+1);
			_clientRevCtrl.print(c);
			_starttime = millis();//reset the reconneting count
			free(c);
			c = NULL;
		}
		else
		{
			Serial.println("malloc:F");
		}
		connStr = "";
		_sensorValueStr = "";
		
		//checkFreeMem();
		
	}
	else 
	{
		// if you didn't get a connection to the server:
		Serial.print("Send");
		Serial.println("Fail");
	}	
}
*/
void LeweiTcpClient::checkFreeMem()
{
	int startPos = 0;
		for(int i = 512;i>0;i--)
		{
			char* c = (char*)malloc(i+startPos);
			if(c)
			{
				free(c);
				c=NULL;
				Serial.print("M");
				Serial.println(i+startPos);
				break;
			}
		}
}

/*
void LeweiTcpClient::sendSensorValue(String sensorName,int sensorValue)
{
	sendSensorValue(sensorName,String(sensorValue));
}


void LeweiTcpClient::sendSensorValue(String sensorName,float sensorValue)
{
	char s[10];
	int val_len = snprintf(s, 10,"%d.%02u", (int)sensorValue, (int)(abs(sensorValue)*100+0.5) % 100);
	sendSensorValue(sensorName,(String)s);
	
}

void LeweiTcpClient::sendSensorValue(String sensorName,double sensorValue)
{
	char s[10];
	int val_len = snprintf(s, 10,"%d.%02u", (int)sensorValue, (int)(abs(sensorValue)*100+0.5) % 100);
	sendSensorValue(sensorName,(String)s);
	
}
void LeweiTcpClient::sendSensorValue(String sensorName,long sensorValue)
{
	sendSensorValue(sensorName,(String)sensorValue);
	
}
*/

void LeweiTcpClient::execute(void (*callfuct)())
{
	//Serial.println("exec no para");
    callfuct();
}
void LeweiTcpClient::execute(void (*callfuct)(char*),char* p1)
{
    callfuct(p1);
}
void LeweiTcpClient::execute(void (*callfuct)(char*,char*),char* p1,char* p2)
{
    callfuct(p1,p2);
}

/*
void LeweiTcpClient::execute(void (*callfuct)(char*,char*,char*),char* p1,char* p2,char* p3)
{
    callfuct(p1,p2,p3);
}

void LeweiTcpClient::execute(void (*callfuct)(char*,char*,char*,char*),char* p1,char* p2,char* p3,char* p4)
{
    callfuct(p1,p2,p3,p4);
}
void LeweiTcpClient::execute(void (*callfuct)(char*,char*,char*,char*,char*),char* p1,char* p2,char* p3,char* p4,char* p5)
{
    callfuct(p1,p2,p3,p4,p5);
}
*/

/*

void LeweiTcpClient::addUserFunction(UserFunction &uFunction)
{
	UserFunctionNode *n1,*n2;
	n2 = (UserFunctionNode*) new(UserFunctionNode);
	
	
	if(uFunction.userFunctionAddr5!=NULL)
	{
		//Serial.println("reg addr5.");
		n2->userFunctionAddr5 = uFunction.userFunctionAddr5;
	}
	else 
		if(uFunction.userFunctionAddr4!=NULL)
	{
		//Serial.println("reg addr4.");
		n2->userFunctionAddr4 = uFunction.userFunctionAddr4;
	}
	else 
		if(uFunction.userFunctionAddr3!=NULL)
	{
		//Serial.println("reg addr3.");
		n2->userFunctionAddr3 = uFunction.userFunctionAddr3;
	}
	else 
		if(uFunction.userFunctionAddr2!=NULL)
	{
		//Serial.println("reg addr2.");
		n2->userFunctionAddr2 = uFunction.userFunctionAddr2;
	}
	else 
	if(uFunction.userFunctionAddr1!=NULL)
	{
		//Serial.println("reg addr1.");
		n2->userFunctionAddr1 = uFunction.userFunctionAddr1;
	}
	else if(uFunction.userFunctionAddr0!=NULL)
	{
		//Serial.println("reg addr0.");
		n2->userFunctionAddr0 = uFunction.userFunctionAddr0;
	}
	
	n2->userFunctionName = uFunction.userFunctionName;
		//Serial.println(uFunction.userFunctionName);
	if(head==NULL)
	{
		head = n2;
		n2->next = NULL;
	}
	else
	{
		n1 = head;
		while(n1->next != NULL)
		{
			n1= n1->next;
		}
		n1->next = n2;
		n2->next = NULL;
	}
}

*/

void LeweiTcpClient::addUserSwitch(UserSwitch &uSwitch)
{
	UserSwitchNode *n1,*n2;
	n2 = (UserSwitchNode*) new(UserSwitchNode);
	
	//n2->userSwitchOnFunctionAddr = uSwitch.userSwitchOnFunctionAddr;
	//n2->userSwitchOffFunctionAddr = uSwitch.userSwitchOffFunctionAddr;
	n2->userSwitchFunctionAddr = uSwitch.userSwitchFunctionAddr;
	
	n2->userSwitchId = uSwitch.userSwitchId;
	n2->userSwitchState = uSwitch.userSwitchState;
	
	if(switchHead==NULL)
	{
		switchHead = n2;
		n2->next = NULL;
	}
	else
	{
		n1 = switchHead;
		while(n1->next != NULL)
		{
			n1= n1->next;
		}
		n1->next = n2;
		n2->next = NULL;
	}
}
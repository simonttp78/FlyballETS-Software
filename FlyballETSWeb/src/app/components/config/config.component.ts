import { Component, OnInit } from '@angular/core';
import { WebsocketService } from '../../services/websocket.service';
import { WebsocketDataRequest } from '../../interfaces/websocket-data-request';
import { ConfigArray } from '../../interfaces/config-array';
import { ConfigData } from '../../class/config-data';


@Component({
  selector: 'app-config',
  templateUrl: './config.component.html',
  styleUrls: ['./config.component.css']
})
export class ConfigComponent implements OnInit {

  isConnected: boolean;
  sessionEnded: boolean;
  submitted: boolean;

  configData = new ConfigData("","");

  constructor(private configDataService:WebsocketService) {
    this.configDataService = configDataService;
    this.initiateConnection();
  }

  ngOnInit() {
     this.isConnected = false;
     this.sessionEnded = false;
     this.submitted = false;
     this.requestConfigData();
  }

  handleConfigData(newConfigData) {
    this.configData = newConfigData;
    console.log(this.configData);
  }

  requestConfigData() {
    let request:WebsocketDataRequest = {
      getData: "config"
    };
    this.configDataService.getData(request);
  }

  handleConfigResult(configResult) {
    this.submitted = false;
    if(configResult.success) {
      console.log("Config saved successfully!");
    }
  }

  onSubmit() {
    let newConfigArray:ConfigArray = {
      config: [
        {name: "APName", value: this.configData.APName},
        {name: "APPass", value: this.configData.APPass}
      ]
    };
    console.log(newConfigArray);
    this.submitted = true;
    this.configDataService.sendConfig(newConfigArray);
    console.log("new config sent!");
  }
  
  initiateConnection() {
    console.log("Attempting WS connection!");
    this.isConnected = false;
    this.sessionEnded = false;
    this.configDataService.dataStream.subscribe(
      msg => {
        this.isConnected = true;
        console.log("received data: ");
        console.log(msg);
        if(msg.dataResult && msg.dataResult.success && msg.dataResult.configData) {
          this.handleConfigData(msg.dataResult.configData);
        } else if (msg.configResult) {
          this.handleConfigResult(msg.configResult);
        }
      },
      err => {
        this.isConnected = false;
        this.initiateConnection();  //Retry connection since this was unexpected
      },
      () => {
        this.sessionEnded = true; 
      }
    )
  }
}

import { Component, OnInit } from '@angular/core';

import { Race } from '../../interfaces/race';
import { WebsocketService } from '../../services/websocket.service';
import { WebsocketAction } from '../../interfaces/websocketaction';

@Component({
  selector: 'app-racedisplay',
  templateUrl: './racedisplay.component.html',
  styleUrls: ['./racedisplay.component.css'],
  providers: [WebsocketService]
})
export class RacedisplayComponent implements OnInit {

  currentRace:Race = {
    id: 0,
    startTime: 0,
    endTime: 0,
    elapsedTime: 0,
    raceState: 0,
    raceStateFriendly: "Stopped",
    dogData: []
  };

  startDisabled: boolean;
  stopDisabled: boolean;
  resetDisabled: boolean;

  isConnected: boolean;
  sessionEnded: boolean;

  raceDataService = new WebsocketService('ws://' + window.location.host + '/ws');

  constructor() {
    this.initiateConnection();
  }

  ngOnInit() {
     this.isConnected = false;
     this.sessionEnded = false;
  }

  startRace() {
     console.log('starting race');
     let StartAction:WebsocketAction = {
       action: "StartRace"
      };
     this.raceDataService.sendAction(StartAction);
  }

  stopRace() {
    console.log('stopping race');
    let StopAction:WebsocketAction = {
      action: "StopRace"
     };
    this.raceDataService.sendAction(StopAction);
  }
  resetRace() {
    console.log('resetting race');
    let StopAction:WebsocketAction = {
      action: "ResetRace"
     };
    this.raceDataService.sendAction(StopAction);
  }

  HandleCurrentRaceData(raceData:Race) {
    this.currentRace =  raceData;
    
    switch(this.currentRace.raceState) {
      case 0:
        this.currentRace.raceStateFriendly = "Stopped";
        break;
      case 1:
        this.currentRace.raceStateFriendly = "Starting";
        break;
      case 2:
        this.currentRace.raceStateFriendly = "Running";
        break;
    }
    this.startDisabled = !(this.currentRace.raceState == 0 && this.currentRace.startTime == 0);
    this.stopDisabled = (this.currentRace.raceState == 0);
    this.resetDisabled = !(this.currentRace.raceState == 0 && this.currentRace.startTime != 0);
  }

  reconnect() {
    this.initiateConnection();
  }

  initiateConnection() {
    console.log("Attempting WS connection!");
    this.isConnected = false;
    this.sessionEnded = false;
    this.raceDataService.dataStream.subscribe(
      msg => {
        this.isConnected = true;
        //console.log("Response from websocket: ");
        //console.log(msg);
        if(msg.RaceData) {
          this.HandleCurrentRaceData(<Race>msg.RaceData);
        }
      },
      err => {
        //console.log("ws error: ");
        //console.log(err);
        this.isConnected = false;
        this.initiateConnection();  //Retry connection since this was unexpected
      },
      () => {
        console.log("ws closed");
        this.sessionEnded = true; 
      }
    )
  }
}

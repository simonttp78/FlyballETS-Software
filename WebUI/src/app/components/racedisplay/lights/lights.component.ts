import { Component, OnInit, OnDestroy, Input } from '@angular/core';
import { LightStates } from '../../../interfaces/light-states';

@Component({
   selector: 'app-lights',
   templateUrl: './lights.component.html',
   styleUrls: ['./lights.component.scss']
})
export class LightsComponent implements OnInit {
   @Input() lightStates: LightStates;

   constructor() {
   }

   ngOnInit() {
   }

}

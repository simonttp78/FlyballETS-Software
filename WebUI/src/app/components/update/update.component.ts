import { Component } from '@angular/core';
import { HttpClient, HttpRequest, HttpEventType, HttpResponse } from '@angular/common/http';

@Component({
  selector: 'app-update',
  templateUrl: './update.component.html',
  styleUrls: ['./update.component.scss']
})
export class UpdateComponent {
  selectedFile: File = null;
  progress: number = 0;
  updating: boolean = false;

  constructor(private http: HttpClient) { }

  onFileSelected(event: any) {
    this.selectedFile = event.target.files[0];
  }

  triggerUpdate() {
    if (!this.selectedFile) {
      console.error('No file selected!');
      return;
    }

    const formData = new FormData();
    formData.append('firmware', this.selectedFile, this.selectedFile.name);

    const req = new HttpRequest('POST', 'http://192.168.20.1/update', formData, {
      reportProgress: true // Enable progress tracking
    });

    this.updating = true;
    this.http.request(req).subscribe(
      (event) => {
        if (event.type === HttpEventType.UploadProgress) {
          this.progress = Math.round(100 * event.loaded / event.total);
        } else if (event instanceof HttpResponse) {
          console.log('OTA Update initiated', event.body);
          this.updating = false;
          this.progress = 0;
        }
      },
      (error) => {
        console.error('Error initiating OTA update:', error);
        this.updating = false;
      }
    );
  }
}
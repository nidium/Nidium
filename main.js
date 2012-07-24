/*  canvas.fillRect(0,0,150,150);  
  canvas.translate(75,75);  
  
  // Create a circular clipping path  
  canvas.beginPath();  
  canvas.arc(0,0,60,0,Math.PI*2,true);  
  canvas.clip();  
  

    
  canvas.fillStyle = "blue";  
  canvas.fillRect(-75,-75,150,150);
*/


   canvas.fillStyle = 'rgb(255,221,0)';
  canvas.fillRect(0,0,150,37.5);
  canvas.fillStyle = 'rgb(102,204,0)';
  canvas.fillRect(0,37.5,150,37.5);
  canvas.fillStyle = 'rgb(0,153,255)';
  canvas.fillRect(0,75,150,37.5);
  canvas.fillStyle = 'rgb(255,51,0)';
  canvas.fillRect(0,112.5,150,37.5);

  // Draw semi transparent circles
  for (i=0;i<10;i++){
    canvas.fillStyle = 'rgba(255,255,255,'+(i+1)/10+')';
    for (j=0;j<4;j++){
      canvas.fillRect(5+i*14,5+j*37.5,14,27.5)
    }
  }
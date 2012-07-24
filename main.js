/*  canvas.fillRect(0,0,150,150);  
  canvas.translate(75,75);  
  
  // Create a circular clipping path  
  canvas.beginPath();  
  canvas.arc(0,0,60,0,Math.PI*2,true);  
  canvas.clip();  
  

    
  canvas.fillStyle = "blue";  
  canvas.fillRect(-75,-75,150,150);
*/

  canvas.transform(1, 0, 0, 1., 0, 0);
  canvas.fillStyle = '#FD0';  
  canvas.fillRect(0,0,75,75);

  canvas.fillStyle = '#6C0';  
  canvas.fillRect(75,0,75,75);  
  canvas.fillStyle = '#09F';  
  canvas.fillRect(0,75,75,75);  
  canvas.fillStyle = '#F30';  
  canvas.fillRect(75,75,75,75);  
  canvas.fillStyle = '#FFF';  
  
  // set transparency value  
  canvas.globalAlpha = 0.3;  
  
  // Draw semi transparent circles  
  for (var i=0;i<7;i++){  
      canvas.beginPath();  
      canvas.arc(75,75,10+10*i,0,Math.PI*2,false);  
      canvas.fill();  
  }  
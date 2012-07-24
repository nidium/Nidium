/*  canvas.fillRect(0,0,150,150);  
  canvas.translate(75,75);  
  
  // Create a circular clipping path  
  canvas.beginPath();  
  canvas.arc(0,0,60,0,Math.PI*2,true);  
  canvas.clip();  
  

    
  canvas.fillStyle = "blue";  
  canvas.fillRect(-75,-75,150,150);
*/


  var lineCap = ['butt','round','square'];

  // Draw guides
  canvas.strokeStyle = '#09f';
  canvas.beginPath();
  canvas.moveTo(10,10);
  canvas.lineTo(140,10);
  canvas.moveTo(10,140);
  canvas.lineTo(140,140);
  canvas.stroke();

  // Draw lines
  canvas.strokeStyle = 'black';
  for (i=0;i<lineCap.length;i++){
    canvas.lineWidth = 15;
    canvas.lineCap = lineCap[i];
    canvas.beginPath();
    canvas.moveTo(25+i*50,10);
    canvas.lineTo(25+i*50,140);
    canvas.stroke();
  }
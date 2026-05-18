As a challenge, I created a 3d renderer from scratch. 
Video demo:
https://www.youtube.com/watch?v=_bBBW3rWjpQ

Note this was created with very little additional resources / 3d knowledge so some of the math may be wrong / unconventional / inefficient.

Everything is in 1 file for easy compilation.

Controls are here: 
if      (key == 'w') cam.move(Vector3D(0,0,1) * moveSpeed); // forward 
else if (key == 's') cam.move(Vector3D(0,0,1) * (-moveSpeed)); // back
else if (key == 'a') cam.move(Vector3D(1,0,0) * (-moveSpeed)); // left
else if (key == 'd') cam.move(Vector3D(1,0,0) * (moveSpeed)); // right
else if (key == 'x') cam.move(Vector3D(0,1,0) * moveSpeed); // look down
else if (key == 'c') cam.move(Vector3D(0,1,0) * (-moveSpeed)); // look up
else if (key == 'q') cam.lookLeft(lookSpeed); //look left
else if (key == 'e') cam.lookLeft(-lookSpeed); //look right
else if (key == 'r') cam.lookDown(-lookSpeed);  //look up
else if (key == 'f') cam.lookDown(lookSpeed); //look down
else if (key == 'b') break;     

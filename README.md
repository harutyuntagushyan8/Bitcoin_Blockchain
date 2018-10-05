Small GUI application to estimate your cpu hashrate for SHA256,
find appropriate Nonce for particular block of Bitcoin Blockchain.
This is not fully complete program that can be used to mine bitcoin.It concerned
only finding Nonce for appropriate block.
 
Requirements:  
QT Tools or build with qmake  
On  Linux:  
sudo apt install build-essential (Compiler ,linker, ...)  
sudo apt install qt5-default(This will install qmake and libs)
   sudo apt install make  

Build steps:  
Qmake Nonce.pro  
make  
./Nonce  
Clean All:  
make clean  

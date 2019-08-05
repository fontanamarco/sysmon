# sysmon

sysmon is a system monitor for linux os.
It is a very limited versin of the linux 'top' program.

## To setup and compile in Udacity Ubuntu workspace:

1. Install `ncurses` package
```
sudo apt-get install libncurses5-dev libncursesw5-dev
```
2. Compile and run
```
g++ -std="c++17" main.cpp -lncurses

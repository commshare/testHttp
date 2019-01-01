there=$PWD
cd bin
export LD_LIBRARY_PATH=.
./domoaster -c ../domoaster.conf
cd $there

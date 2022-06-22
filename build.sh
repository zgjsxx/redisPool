workdir=$(cd $(dirname $0); pwd)
hiredisdir=$workdir/hiredis
cd $hiredisdir

make 
make install

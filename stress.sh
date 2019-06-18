echo "$1"

c=1

while [ $c -le $1 ]
do
	./bot $2 $3 &
	(( c++ ))
done

pid=$!

trap 'kill $(jobs -p)' EXIT
wait

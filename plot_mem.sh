name=$(basename $1)

$@ &
pid=$!

trap "kill $pid 2> /dev/null" EXIT

echo "$pid" "$name"

rm /tmp/mem.log

while kill -0 $pid 2> /dev/null; do
	ps -p $pid -o pid=,%mem=,vsz= >> /tmp/mem.log
	gnuplot ./plot_fcn.plt
	sleep 1
done

trap - EXIT

#!/usr/bin/env bash
set -e

cleanup=true
default_benchmarks=$(find ./res/scenes/benchmarks/*.rawscene)

function run_benchmark(){
  ./build/modengine -r $1 -e $3 -l "$2" -q > /dev/null
}

function process_output(){
  program="
    BEGIN{
      trianglezone = 0;
    }

    /^$/ {
      trianglezone = 0;
    }

    //{
      if (trianglezone == 1){
        print \$0;
      }
    }

    /^$2$/ {
      trianglezone = 1;
    }
  "
  cat $1 | awk "$program" > $3
}

function generate_plot(){
  data_file=$1
  output_file=$2
  command="
    set terminal png; 
    set title '$3 vs frametime';
    set xlabel '$3';
    set ylabel 'frametime';
    plot '$data_file' with points pt 7;
  "
  gnuplot -e "$command" > "$output_file"
}

mkdir -p ./build/benchmarks 

benchmarks=$default_benchmarks
specific_benchmark=false

if [[ ! -z "$1" ]]
then
  benchmarks="$1"
  specific_benchmark=true
fi


last_triangle_png="";
last_obj_png=""

benchmark_duration=$((1000 * 5))
if [[ ! -z "$2" ]]
then
  benchmark_duration="$2"
fi

for benchmark in $benchmarks
do
    benchfile="./build/benchmarks/$(basename $benchmark).benchmark"
    tri_benchdat_out="./build/benchmarks/$(basename $benchmark).triangle_benchmark.out"
    tri_benchdat_png="./build/benchmarks/$(basename $benchmark).triangle_benchmark.png"
    obj_benchdat_out="./build/benchmarks/$(basename $benchmark).obj_benchmark.out"
    obj_benchdat_png="./build/benchmarks/$(basename $benchmark).obj_benchmark.png"

    last_triangle_png=$tri_benchdat_png
    last_obj_png=$obj_benchdat_png

    echo "Running Benchmark for $benchmark"
    run_benchmark "$benchmark" "$benchfile" "$benchmark_duration"
    
    process_output "$benchfile" "triangle-count to frametime" "$tri_benchdat_out"
    process_output "$benchfile" "object-count to frametime" "$obj_benchdat_out"

    generate_plot "$tri_benchdat_out" "$tri_benchdat_png" "triangles"
    generate_plot "$obj_benchdat_out" "$obj_benchdat_png" "objects"

    if $cleanup 
    then
      rm $benchfile
      rm $tri_benchdat_out
      rm $obj_benchdat_out
    fi 

    echo "Finished Benchmark for $benchmark"
done

if $specific_benchmark
then
  xdg-open "$last_triangle_png"
  xdg-open "$last_obj_png"
fi 


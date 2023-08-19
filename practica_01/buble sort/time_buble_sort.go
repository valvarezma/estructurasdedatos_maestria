package main

import (
	"encoding/csv"
	"fmt"
	"math/rand"
	"os"
	"time"
)

func bubbleSort(arr []int) {
	n := len(arr)
	for i := 0; i < n-1; i++ {
		for j := 0; j < n-i-1; j++ {
			if arr[j] > arr[j+1] {
				arr[j], arr[j+1] = arr[j+1], arr[j]
			}
		}
	}
}

func measureTimeSorting(algorithm func([]int), listSize int) time.Duration {
	arr := make([]int, listSize)
	rand.Seed(time.Now().UnixNano())
	for i := 0; i < listSize; i++ {
		arr[i] = rand.Intn(100000) + 1
	}

	startTime := time.Now()
	algorithm(arr)
	endTime := time.Now()

	return endTime.Sub(startTime)
}

func exportToCSVGo(results [][]string) error {
	file, err := os.Create("results_go.csv")
	if err != nil {
		return err
	}
	defer file.Close()

	writer := csv.NewWriter(file)
	defer writer.Flush()

	for _, result := range results {
		err := writer.Write(result)
		if err != nil {
			return err
		}
	}

	return nil
}

func main() {
	listSizes := []int{100, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000, 30000, 40000, 50000}

	fmt.Println("Tiempos de ejecución para diferentes tamaños de entrada:")
	fmt.Println("| Tamaño de la lista | Tiempo de ejecución (segundos) |")
	fmt.Println("|--------------------|-------------------------------|")

	results := [][]string{{"Tamaño de la lista", "Tiempo de ejecución (segundos)"}}

	for _, size := range listSizes {
		executionTime := measureTimeSorting(bubbleSort, size)
		result := []string{fmt.Sprintf("%d", size), fmt.Sprintf("%.6f", executionTime.Seconds())}
		results = append(results, result)
		fmt.Printf("| %6d\t\t  | %.6f\t\t   |\n", size, executionTime.Seconds())
	}

	fmt.Println("Resultados almacenados en el slice 'results':")
	fmt.Println(results)

	if err := exportToCSVGo(results); err != nil {
		fmt.Println("Error al exportar a CSV:", err)
	} else {
		fmt.Println("Resultados exportados a results_go.csv")
	}
}

package main

import (
	"encoding/csv"
	"fmt"
	"math/rand"
	"os"
	"time"
)

func selectionSort(arr []int) {
	n := len(arr)
	for i := 0; i < n-1; i++ {
		minIndex := i
		for j := i + 1; j < n; j++ {
			if arr[j] < arr[minIndex] {
				minIndex = j
			}
		}
		arr[i], arr[minIndex] = arr[minIndex], arr[i]
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

func main() {
	listSizes := []int{100, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000, 30000, 40000, 50000}

	fmt.Println("Tiempos de ejecución para diferentes tamaños de entrada:")
	fmt.Println("| Tamaño de la lista | Tiempo de ejecución (segundos) |")
	fmt.Println("|--------------------|-------------------------------|")

	results := [][]string{{"Tamaño de la lista", "Tiempo de ejecución (segundos)"}}

	for _, size := range listSizes {
		executionTime := measureTimeSorting(selectionSort, size)
		result := []string{fmt.Sprintf("%d", size), fmt.Sprintf("%.6f", executionTime.Seconds())}
		results = append(results, result)
		fmt.Printf("| %6d\t\t  | %.6f\t\t   |\n", size, executionTime.Seconds())
	}

	if err := exportToCSVGo(results); err != nil {
		fmt.Println("Error al exportar a CSV:", err)
	} else {
		fmt.Println("Resultados exportados a results_selection_sort.csv")
	}
}

func exportToCSVGo(results [][]string) error {
	file, err := os.Create("results_selection_sort.csv")
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

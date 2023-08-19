package main

import (
	"encoding/csv"
	"fmt"
	"math/rand"
	"os"
	"strconv"
	"time"
)

func merge(arr []int, left int, mid int, right int) {
	leftSize := mid - left + 1
	rightSize := right - mid

	leftHalf := make([]int, leftSize)
	rightHalf := make([]int, rightSize)

	copy(leftHalf, arr[left:left+leftSize])
	copy(rightHalf, arr[mid+1:mid+1+rightSize])

	i, j, k := 0, 0, left

	for i < leftSize && j < rightSize {
		if leftHalf[i] <= rightHalf[j] {
			arr[k] = leftHalf[i]
			i++
		} else {
			arr[k] = rightHalf[j]
			j++
		}
		k++
	}

	for i < leftSize {
		arr[k] = leftHalf[i]
		i++
		k++
	}

	for j < rightSize {
		arr[k] = rightHalf[j]
		j++
		k++
	}
}

func mergeSort(arr []int, left int, right int) {
	if left >= right {
		return
	}

	mid := left + (right-left)/2
	mergeSort(arr, left, mid)
	mergeSort(arr, mid+1, right)
	merge(arr, left, mid, right)
}

func measureTimeSorting(listSize int) time.Duration {
	arr := make([]int, listSize)
	rand.Seed(time.Now().UnixNano())

	for i := 0; i < listSize; i++ {
		arr[i] = rand.Intn(100000) + 1
	}

	startTime := time.Now()
	mergeSort(arr, 0, len(arr)-1)
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
	var results [][]string
	for _, size := range listSizes {
		executionTime := measureTimeSorting(size)
		fmt.Printf("| %6d\t\t  | %.6f\t\t   |\n", size, executionTime.Seconds())
		sizeStr := strconv.Itoa(size)
		timeStr := fmt.Sprintf("%.6f", executionTime.Seconds())
		results = append(results, []string{sizeStr, timeStr})
	}

	fmt.Println("Resultados almacenados en el vector 'results':")
	fmt.Println(results)

	err := exportToCSVGo(results)
	if err != nil {
		fmt.Println("Error al exportar a CSV:", err)
	}
}

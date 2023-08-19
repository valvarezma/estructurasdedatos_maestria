import random
import time
import csv

def export_to_csv_python(results):
    with open('results_python.csv', 'w', newline='') as csvfile:
        fieldnames = ['Tamaño de la lista', 'Tiempo de ejecución (segundos)']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        
        for size, time in results:
            writer.writerow({'Tamaño de la lista': size, 'Tiempo de ejecución (segundos)': time})

def merge_sort(arr):
    if len(arr) <= 1:
        return arr

    mid = len(arr) // 2
    left_half = merge_sort(arr[:mid])
    right_half = merge_sort(arr[mid:])

    return merge(left_half, right_half)

def merge(left, right):
    merged = []
    left_idx, right_idx = 0, 0

    while left_idx < len(left) and right_idx < len(right):
        if left[left_idx] < right[right_idx]:
            merged.append(left[left_idx])
            left_idx += 1
        else:
            merged.append(right[right_idx])
            right_idx += 1

    merged.extend(left[left_idx:])
    merged.extend(right[right_idx:])
    return merged

def measure_time_sorting(list_size):
    # Generar lista de números desordenados aleatorios
    arr = [random.randint(1, 100000) for _ in range(list_size)]

    # Medir tiempo de ejecución
    start_time = time.time()
    sorted_arr = merge_sort(arr)
    end_time = time.time()

    return end_time - start_time

if __name__ == "__main__":
    list_sizes = [100, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000, 30000, 40000, 50000]

    print("Tiempos de ejecución para diferentes tamaños de entrada:")
    print("| Tamaño de la lista | Tiempo de ejecución (segundos) |")
    print("|--------------------|-------------------------------|")
    results = []
    for size in list_sizes:
        execution_time = measure_time_sorting(size)
        results.append((size, execution_time))
        print(f"| {size:19} | {execution_time:29} |")
    print("Resultados almacenados en la lista 'results':")
    print(results)
    #export_to_csv_python(results)
    export_to_csv_python(results)

 
import numpy as np
import matplotlib.pyplot as plt
import time



# Definimos una matriz con las distancias entre las 5 ciudades
# La matriz es simétrica y tiene ceros en la diagonal
distancias = np.array([[0, 113, 56, 167, 147],
                       [113, 0, 137, 142, 98],
                       [56, 137, 0, 133, 135],
                       [167, 142, 133, 0, 58],
                       [147, 98, 135, 58, 0]])

                       
# Elegimos una ciudad de inicio (por ejemplo, la primera)
inicio = int(input("Nodo inicio: "))

ini_time = time.time()

# Creamos una lista para almacenar la ruta
ruta = [inicio]

# Creamos una variable para almacenar el costo total
costo = 0

# Creamos un conjunto para marcar las ciudades visitadas
visitadas = {inicio}

# Repetimos hasta que hayamos visitado todas las ciudades
while len(visitadas) < len(distancias):
    # Obtenemos la fila de la matriz correspondiente a la ciudad actual
    fila = distancias[inicio]

    # Creamos una copia de la fila para no modificar la matriz original
    fila = np.copy(fila)

    # Asignamos un valor muy grande a las ciudades ya visitadas para descartarlas
    for i in visitadas:
        fila[i] = 9999

    # Buscamos el índice del valor mínimo en la fila
    # Este índice corresponde a la ciudad más cercana
    minimo = np.argmin(fila)

    # Añadimos esta ciudad a la ruta
    ruta.append(minimo)

    # Añadimos el costo de ir a esta ciudad al costo total
    costo += fila[minimo]

    # Marcamos esta ciudad como visitada
    visitadas.add(minimo)

    # Actualizamos la ciudad actual con la ciudad más cercana
    inicio = minimo

# Añadimos el costo de regresar a la ciudad de inicio al costo total
costo += distancias[inicio][ruta[0]]

# Añadimos la ciudad de inicio al final de la ruta para cerrar el ciclo
ruta.append(ruta[0])

# Imprimimos la ruta y el costo por pantalla
print("La ruta más corta es:", ruta)
print("El costo total es:", costo)

# Medimos el tiempo de ejecución del código con el módulo timeit
end_time = time.time()

tiempo = end_time - ini_time

# Imprimimos el resultado por pantalla
print("El tiempo de ejecución del algoritmo es: {:.2f}".format(tiempo*1000))

# Usamos la función scatter para dibujar los puntos
plt.scatter(distancias[:, 0], distancias[:, 1], s=100, c='red')

# Usamos un bucle for para dibujar las líneas
for i in range(len(distancias)):
    for j in range(i + 1, len(distancias)):
        plt.plot([distancias[i][0], distancias[j][0]], [distancias[i][1], distancias[j][1]], color='black', alpha=0.3)

# Usamos la función plot para dibujar la ruta más corta
plt.plot(distancias[ruta, 0], distancias[ruta, 1], marker='o', color='blue')

plt.annotate(str(distancias[0][1]), xy=((distancias[0][0] + distancias[1][0]) / 2, (distancias[0][1] + distancias[1][1]) / 2), xytext=(0, 5), textcoords='offset points', ha='center', va='bottom')
plt.annotate(str(distancias[0][2]), xy=((distancias[0][0] + distancias[2][0]) / 2, (distancias[0][1] + distancias[2][1]) / 2), xytext=(0, 5), textcoords='offset points', ha='center', va='bottom')
plt.annotate(str(distancias[0][3]), xy=((distancias[0][0] + distancias[3][0]) / 2, (distancias[0][1] + distancias[3][1]) / 2), xytext=(0, 5), textcoords='offset points', ha='center', va='bottom')
plt.annotate(str(distancias[0][4]), xy=((distancias[0][0] + distancias[4][0]) / 2, (distancias[0][1] + distancias[4][1]) / 2), xytext=(0, 5), textcoords='offset points', ha='center', va='bottom')
plt.annotate(str(distancias[1][2]), xy=((distancias[1][0] + distancias[2][0]) / 2, (distancias[1][1] + distancias[2][1]) / 2), xytext=(0, 5), textcoords='offset points', ha='center', va='bottom')
plt.annotate(str(distancias[1][3]), xy=((distancias[1][0] + distancias[3][0]) / 2, (distancias[1][1] + distancias[3][1]) / 2), xytext=(0, 5), textcoords='offset points', ha='center', va='bottom')
plt.annotate(str(distancias[1][4]), xy=((distancias[1][0] + distancias[4][0]) / 2, (distancias[1][1] + distancias[4][1]) / 2), xytext=(0, 5), textcoords='offset points', ha='center', va='bottom')
plt.annotate(str(distancias[2][3]), xy=((distancias[2][0] + distancias[3][0]) / 2, (distancias[2][1] + distancias[3][1]) / 2), xytext=(0, 5), textcoords='offset points', ha='center', va='bottom')
plt.annotate(str(distancias[2][4]), xy=((distancias[2][0] + distancias[4][0]) / 2, (distancias[2][1] + distancias[4][1]) / 2), xytext=(0, 5), textcoords='offset points', ha='center', va='bottom')
plt.annotate(str(distancias[3][4]), xy=((distancias[3][0] + distancias[4][0]) / 2, (distancias[3][1] + distancias[4][1]) / 2), xytext=(0, 5), textcoords='offset points', ha='center', va='bottom')

plt.show()



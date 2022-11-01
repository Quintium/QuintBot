import numpy as np
import pygame

def parseOutput():
    data = np.zeros(shape)

    with open("QuintTest.out", "r") as file:
        lines = file.readlines()

        for line in lines:
            if line.startswith("Engine match: "):
                vs = line.index(" vs ")
                name = line[vs + 4 : line.rfind(":")]
                argString = name[name.index("_") + 1 :]
                args = [int(arg) for arg in argString.split("_")]
                indices = [dimensions[i].index(n) for i, n in enumerate(args)]

            if line.startswith("Elo difference: "):
                eloDiff = float(line[17:])
                data[tuple(indices)] = eloDiff

    return data

def draw():
    screen.fill((255, 255, 255))
    for x in range(data.shape[0]):
        for y in range(data.shape[1]):
            eloDiff = data[tuple([x, y] + sliders)]
            score = (eloDiff - min) / (max - min)
            color = (255 - 255 * score, 255 * score, 0)
            pygame.draw.rect(screen, color, (x * imWidth / shape[0], (data.shape[1] - 1 - y) * imHeight / shape[1], imWidth / shape[0], imHeight / shape[1]))

    l = imWidth / (len(sliders) + 1)
    d = l / (len(sliders) + 1)

    for i in range(len(sliders)):
        pygame.draw.line(screen, (0, 0, 0), ((i + 1) * d + i * l, imHeight + 50), ((i + 1) * d + (i + 1) * l, imHeight + 50), 3)
        pygame.draw.circle(screen, (100, 100, 100), ((i + 1) * d + (i + sliders[i] / (shape[i + 2] - 1)) * l, imHeight + 50), 8)

    triangleX = (curSlider + 1) * d + (curSlider + 0.5) * l
    pygame.draw.polygon(screen, (0, 255, 0), ((triangleX, imHeight + 70), (triangleX + 6, imHeight + 76), (triangleX - 6, imHeight + 76)))

    pygame.display.flip()

pygame.init()
imWidth = 600
imHeight = 600
screen = pygame.display.set_mode((imWidth, imHeight + 100))

dimensions = [[0, 300], [0, 300], [0, 300], [0, 300], [0, 300]]
shape = [len(dim) for dim in dimensions]
data = parseOutput()
min = np.amin(data)
max = np.amax(data)
print(f"Minimum elo diff: {min}")
print(f"Maximum elo diff: {max}")

sliders = [0 for i in range(data.ndim - 2)]
curSlider = 0

while True:
    for e in pygame.event.get():
        if e.type == pygame.QUIT:
            pygame.quit()
            quit()
        if e.type == pygame.KEYDOWN:
            if e.key == pygame.K_LEFT:
                if curSlider > 0:
                    curSlider -= 1
            if e.key == pygame.K_RIGHT:
                if curSlider < len(sliders) - 1:
                    curSlider += 1
            if e.key == pygame.K_DOWN:
                if sliders[curSlider] > 0:
                    sliders[curSlider] -= 1
            if e.key == pygame.K_UP:
                if sliders[curSlider] < shape[curSlider + 2] - 1:
                    sliders[curSlider] += 1

    draw()

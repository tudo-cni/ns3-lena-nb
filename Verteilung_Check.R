setwd("D:/Git/ubuntu/ns-3-nbiot")
input = read.csv("output.csv", header = FALSE)

hist(input[,2],breaks = 50)
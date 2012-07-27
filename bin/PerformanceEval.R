#!/usr/bin/env Rscript
# load libraries
library(plyr)

# read the csv directory from the arguments list
args <- commandArgs(trailingOnly = TRUE)
csv.dir <- tail(args, 1)

# read the csv files and put them in a list
filenames <- list.files(pattern="\\.csv", path=csv.dir, full.names=TRUE)
csv.data <- ldply(llply(filenames, read.csv))

seeders.70 <- ggplot(data=csv.data[csv.data$Run < 30,])
seeders.30 <- ggplot(data=csv.data[csv.data$Run >= 30,])
seeders.30 + geom_line(aes(x=Elapsed.Time, y=Events.per.Second)) + facet_grid( Run ~ .)

qplot(Elapsed.Time, Events.per.Second,data=csv.data[csv.data$Run >9 || csv.data$Run<20,], color=Run, geom="line")

# unload libraries
detach("package:plyr")
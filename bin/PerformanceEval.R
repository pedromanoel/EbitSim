#!/usr/bin/env Rscript
# process logs created with the commands below

# Create the csv files from the out.log files in the current folder 
# $ bin/process_out
# Add the column Log and Nao
# sed -i'' -n '{/[0-9]$/s/\([0-9]\)$/\1,Nao/p;/Run/s/$/,"Log"/p}' .csv
# Add the column Log and Nao
# sed -i'' -n '{/[0-9]$/s/\([0-9]\)$/\1,Sim/p;/Run/s/$/,"Log"/p}' .csv
# load libraries
library(plyr)
library(ggplot2)

# read the csv directory from the arguments list
args <- commandArgs(trailingOnly = TRUE)
csv.dir <- tail(args, 1)

# read the csv files and put them in a list
filenames <- list.files(pattern="\\.csv", path=csv.dir, full.names=TRUE)
csv.data <- ldply(llply(filenames, read.csv))

# transform into factors
csv.data$Run <- factor(csv.data$Run)

# selection vectors
debug.nolog <- csv.data$Run %in% factor(c(0,2,4)) & csv.data$Log == "Nao"
release.nolog <- csv.data$Run %in% factor(c(1,3,5)) & csv.data$Log == "Nao"
debug.log <- csv.data$Run %in% factor(c(0,2,4)) & csv.data$Log == "Sim"
release.log <- csv.data$Run %in% factor(c(1,3,5)) & csv.data$Log == "Sim"

csv.data[debug.nolog,]

p.debug.nolog <- ggplot(csv.data[debug.nolog,])
p.release.nolog <- ggplot(csv.data[release.nolog,])
p.debug.log <- ggplot(csv.data[debug.log,])
p.release.log <- ggplot(csv.data[release.log,])

# the graphs
boxplot <- geom_boxplot(aes(Run, Events.per.Second, colour=Run), outlier.shape=4, outlier.size = 1)
line.graph <- geom_line(aes(Elapsed.Time, Events.per.Second, colour=Run), size=.3)

boxplot.debug <- p.debug + boxplot +
  scale_colour_hue(name="Experimentos", breaks=c(0,2,4), labels=c("ES-35-10", "ES-92-10", "ES-512-10")) +
  opts(title="\n", axis.title.y=theme_blank(), axis.text.y=theme_blank()) + xlab("")
line.debug <- p.debug + line.graph + ylab("Eventos/s") + xlab("Tempo de execução (s)") +
  scale_colour_hue(name="Experimentos", breaks=c(0,2,4),
    labels=c("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", "", "")) +
  opts(title="Eventos por segundo no modo depuração, sem log\n")

boxplot.release <- p.release + boxplot +
		scale_colour_hue(name="Experimentos", breaks=c(1,3,5), labels=c("ES-35-10", "ES-92-10", "ES-512-10")) +
		opts(title="\n", axis.title.y=theme_blank(), axis.text.y=theme_blank()) + xlab("")
line.release <- p.release + line.graph + ylab("Eventos/s") + xlab("Tempo de execução (s)") +
		scale_colour_hue(name="Experimentos", breaks=c(1,3,5),
				labels=c("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", "", "")) +
		opts(title="Eventos por segundo no modo produção, sem log\n")

line.debug
ggsave(file="line.debug.nolog.png", dpi=300, width=8, height=6)
boxplot.debug
ggsave(file="boxplot.debug.nolog.png", dpi=300, width=3, height=6)

line.release
ggsave(file="line.release.nolog.png", dpi=300, width=8, height=6)
boxplot.release
ggsave(file="boxplot.release.nolog.png", dpi=300, width=3, height=6)

# unload libraries
detach("package:plyr")
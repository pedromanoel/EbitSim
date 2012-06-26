#!/usr/bin/env Rscript
initial.options <- commandArgs(trailingOnly = TRUE)
#file.arg.name <- "--file="
#script.name <- sub(file.arg.name, "", initial.options[grep(file.arg.name, initial.options)])
#script.basename <- dirname(script.name)
csv.file <- tail(initial.options, 1)
#print(initial.options)

# load the necessary libraries
library(ggplot2)
library(reshape)
library(scales)
library(vcd)
csv.file<-"../simulations/results/csv/SimpleTopology-0.csv"
# load the dataset
csv.data <- read.csv (file=csv.file,  na.strings = "NA", nrows = -1, skip = 0, check.names = TRUE, strip.white = FALSE, blank.lines.skip = TRUE)
piece.download <- melt(data=csv.data, id=c("Time"), na.rm=TRUE)
names(piece.download) <- c("Time", "Peer", "Piece")

max.freq <- length(levels(piece.download$Peer))

piece.download <- ddply(piece.download, .(Piece), function(df) cbind(df[order(df$Time), c("Time","Peer")], Freq=1:max.freq/max.freq))
p <-ggplot(data=piece.download) + theme_bw()
p + geom_bar(aes(Time)) + scale_colour_gradientn(colours=heat_hcl(7)) + opts(plot.title="Distribution of the piece request times")
p +
    geom_rect(fill="black", linetype=0, xmin=0, xmax=800, ymin=0, ymax=max(piece.download$Time)) +
    geom_bar(stat="identity", aes(Piece, Time),
             colour=alpha("white", 1/max.freq),
             linetype=1,
             fill=alpha("white", 1/max.freq),
             position="identity") + coord_flip()# + opts(panel.background=theme_rect(fill="purple"))
ggsave(file = "../simulations/results/output.pdf")
dev.off()
#
#
#
#p + geom_rect(colour="black", xmin=0, xmax=max(piece.download$Time), ymin=0, ymax=80) + geom_area(stat="identity", aes(Piece, Time, group=Peer), fill=alpha("white", 1/max.freq), linetype=1, position="identity") + coord_flip()
#
#alpha=1/length(levels(piece.download$Peer))
#
#
#p + geom_bar(stat="identity", colour=alpha("black", alpha),  position="identity")
#p + geom_area(aes(colour=variable, alpha=alpha), position="identity")
#p + geom_line(aes(group=variable, alpha = 1/length(levels(piece.download$variable))))
#p + geom_line(aes(group=variable, alpha = 1/length(levels(piece.download$variable))))
#p + geom_area(aes(group=variable), fill=)

# unload the libraries
detach("package:ggplot2")
detach("package:reshape")
detach("package:scales")
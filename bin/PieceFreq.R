# store the current directory
initial.dir<-getwd()

# change to the new directory
results.dir="/home/pevangelista/Workspaces/workspace-omnetpp/EbitSim/simulations/results/csv/"
setwd(results.dir)

# load the necessary libraries
library(ggplot2)
library(reshape)
library(scales)
library(vcd)

# load the dataset
csv.file <- "SimpleTopology-3.csv"
csv.data <- read.csv (file=csv.file,  na.strings = "NA", nrows = -1, skip = 0, check.names = TRUE, strip.white = FALSE, blank.lines.skip = TRUE)
piece.download <- melt(data=csv.data, id=c("Time"), na.rm=TRUE)
names(piece.download) <- c("Time", "Peer", "Piece")

max.freq <- length(levels(piece.download$Peer))

piece.download <- ddply(piece.download, .(Piece), function(df) cbind(df[order(df$Time), c("Time","Peer")], Freq=1:max.freq/max.freq))
p <-ggplot(data=piece.download) + theme_bw()
p + geom_point(aes(Piece, Time, colour=Freq)) + coord_flip() + scale_colour_gradientn(colours=heat_hcl(7))
p + geom_bar(stat="identity", aes(Piece, Time), colour="black", linetype=0, fill=alpha("red", 1/max.freq), position="identity") + coord_flip()



p + geom_rect(colour="black", xmin=0, xmax=max(piece.download$Time), ymin=0, ymax=80) + geom_area(stat="identity", aes(Piece, Time, group=Peer), fill=alpha("white", 1/max.freq), linetype=1, position="identity") + coord_flip()

alpha=1/length(levels(piece.download$Peer))


p + geom_bar(stat="identity", colour=alpha("black", alpha),  position="identity")
p + geom_area(aes(colour=variable, alpha=alpha), position="identity")
p + geom_line(aes(group=variable, alpha = 1/length(levels(piece.download$variable))))
p + geom_line(aes(group=variable, alpha = 1/length(levels(piece.download$variable))))
p + geom_area(aes(group=variable), fill=)

# unload the libraries
detach("package:ggplot2")
detach("package:reshape")
detach("package:scales")

# change back to the original directory
setwd(initial.dir)

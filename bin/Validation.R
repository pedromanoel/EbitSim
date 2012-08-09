#!/usr/bin/env Rscript
# load libraries

## Summarizes data.
## Gives count, mean, standard deviation, standard error of the mean, and confidence interval (default 95%).
##   data: a data frame.
##   measurevar: the name of a column that contains the variable to be summariezed
##   groupvars: a vector containing names of columns that contain grouping variables
##   na.rm: a boolean that indicates whether to ignore NA's
##   conf.interval: the percent range of the confidence interval (default is 95%)
summarySE <- function(data=NULL, measurevar, groupvars=NULL, na.rm=FALSE,
		conf.interval=.95, .drop=TRUE) {
	require(plyr)
	
	# New version of length which can handle NA's: if na.rm==T, don't count them
	length2 <- function (x, na.rm=FALSE) {
		if (na.rm) sum(!is.na(x))
		else       length(x)
	}
	
	# This is does the summary; it's not easy to understand...
	datac <- ddply(data, groupvars, .drop=.drop,
			.fun= function(xx, col, na.rm) {
				c( N    = length2(xx[,col], na.rm=na.rm),
						mean = mean   (xx[,col], na.rm=na.rm),
						sd   = sd     (xx[,col], na.rm=na.rm)
				)
			},
			measurevar,
			na.rm
	)
	
	# Rename the "mean" column    
	datac <- rename(datac, c("mean"=measurevar))
	
	datac$se <- datac$sd / sqrt(datac$N)  # Calculate standard error of the mean
	
	# Confidence interval multiplier for standard error
	# Calculate t-statistic for confidence interval: 
	# e.g., if conf.interval is .95, use .975 (above/below), and use df=N-1
	ciMult <- qt(conf.interval/2 + .5, datac$N-1)
	datac$ci <- datac$se * ciMult
	
	return(datac)
}

library(plyr)
library(ggplot2)

# read the csv directory from the arguments list
args <- commandArgs(trailingOnly = TRUE)
csv.dir <- tail(args, 1)

# read the szydlowsky data
filenames <- list.files(pattern="\\.txt", path=csv.dir, full.names=TRUE)
csv.data <- ldply(llply(filenames, read.csv))

# read the simulation data
filenames <- list.files(pattern="small.*\\.csv$", path=csv.dir, full.names=TRUE)
csv.data.sim.small <- ldply(llply(filenames, read.csv))
filenames <- list.files(pattern="medium.*\\.csv$", path=csv.dir, full.names=TRUE)
csv.data.sim.medium <- ldply(llply(filenames, read.csv))
filenames <- list.files(pattern="large.*\\.csv$", path=csv.dir, full.names=TRUE)
csv.data.sim.large <- ldply(llply(filenames, read.csv))

#filenames <- list.files(pattern="sim-.*\\.csv$", path=csv.dir, full.names=TRUE)
#csv.data.sim.small.2 <- ldply(llply(filenames, read.csv))

filenames <- list.files(pattern="messages-.*\\.csv$", path=csv.dir, full.names=TRUE)
csv.data.sim.messages <- ldply(llply(filenames, read.csv))

# merge all simulation data
csv.data.sim <- rbind(csv.data.sim.small, csv.data.sim.medium,
		csv.data.sim.large)#, csv.data.sim.small.2)

# generate the empirical cdf and put it in the column Probability
csv.data.sim <- ddply(csv.data.sim,.(Experiment),transform,
		Probability = ecdf(Download.Time)(Download.Time))

# get only the columns I want
sim.cdf <- csv.data.sim[,c("Download.Time", "Probability", "Experiment", "Client")]
names(sim.cdf) <- c("Time", "Probability", "Experiment", "Client")

# merge the new data with the experiment data
csv.data.all <- rbind(csv.data,sim.cdf)

# select pallete
my.cols <- brewer.pal(4, "Set1")
my.cols[4] <- "#000000" # change the last color to black

ggplot(csv.data[csv.data$Experiment == "cl-35-5",],
       aes(Time, Probability, colour=Client, linetype=Experiment)) +
	geom_line() + # plot normal
	# plot ebitsim
	geom_line(data=sim.cdf[sim.cdf$Experiment=="es-35-5",], size=1) + 
	scale_size_manual(values=c("solid", "dashed"))

# get the cluster experiments only
d <- csv.data.all[grep("cl", csv.data.all$Experiment),]
d <- d[d$Client != "EbitSim2",]
p <- ggplot(data=d)
p + geom_line(aes(x=Time,y=Probability,color=Client)) +
		facet_grid (Experiment ~ .) +
		opts(title="Comparação de desempenho\nentre os experimentos e a simulação") +
		xlab("Tempo de Download (s)") + ylab("Probabilidade") +
		scale_colour_hue(name="Cliente")



d <- csv.data.all[grep("cl", csv.data.all$Experiment),]
d <- d[d$Client == "EbitSim" | d$Client == "mainline", ]
d <- d[d$Experiment == "cl-92-5", ]
p <- ggplot(data=d)
p <- p + geom_line(aes(x=Time,y=Probability,color=Client)) + facet_grid (Experiment ~ .)
p + opts(title="Desempenho dos Clientes BitTorrent\n") +
		xlab("Tempo de Download (s)") + ylab("Probabilidade") +
		scale_colour_hue(name="Cliente")
ggsave("experimento_cdf.png", dpi=300)

seeders.70 <- ggplot(data=csv.data[csv.data$Run < 30,])
seeders.30 <- ggplot(data=csv.data[csv.data$Run >= 30,])
seeders.30 + geom_line(aes(x=Elapsed.Time, y=Events.per.Second)) + facet_grid( Run ~ .)

qplot(Elapsed.Time, Events.per.Second,data=csv.data[csv.data$Run >9 || csv.data$Run<20,], color=Run, geom="line")

# unload libraries
detach("package:plyr")
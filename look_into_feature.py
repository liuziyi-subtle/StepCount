from tsfresh.feature_extraction.feature_calculators import partial_autocorrelation, agg_autocorrelation

x = [
17903616, 11350016, 16138240, 10080256, 14114816, 19898368,
       17555456, 24993792, 16850944, 17031168, 14340096, 12206080,
       19550208, 13406208, 14213120, 13639680, 14893056, 12636160,
       18550784, 19963904, 23650304, 22077440, 18305024, 13099008,
       12083200, 10059776, 10633216,  8855552, 16654336, 19681280,
       12062720, 34144256, 15900672, 14917632, 12492800, 14188544,
       13737984, 15544320, 12738560,  9965568, 10084352, 12902400,
       12181504, 14188544, 12996608, 13131776, 15114240, 16785408,
       13651968, 12455936, 13623296,  9900032, 12263424, 11784192,
       12271616, 15343616, 16982016, 15405056, 15077376, 14716928,
       15392768, 13389824, 16994304, 16470016, 14094336, 16154624,
       16199680, 11718656, 16523264, 19718144, 17326080, 19337216,
       14061568, 15290368, 16625664, 13635584, 15360000, 14766080,
       15060992, 15941632, 16633856, 14962688, 14721024, 13885440,
       14127104, 15028224, 14962688, 15704064, 16351232, 15482880,
       15872000, 15273984, 14651392, 14458880, 14819328, 15241216,
       15048704, 15478784, 16470016, 15224832, 15224832, 15020032,
       14819328, 14782464, 14606336, 14606336, 14819328, 15417344,
       15273984, 15650816, 15618048, 15405056, 14376960, 14405632,
       14626816, 14524416, 15257600, 14864384, 14458880, 15847424,
       15261696, 15085568, 15163392, 15192064, 15118336
]

# a = partial_autocorrelation(x, [{'lag': 2}])
a = agg_autocorrelation(x, [{'maxlag': 40, 'f_agg': 'var'}])

print()

print()
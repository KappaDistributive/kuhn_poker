import pandas as pd
import seaborn as sns

from matplotlib import pyplot as plt

if __name__ == "__main__":
  data = []
  with open("data/kuhn__vs__counter_exploit.txt") as f:
    for round, line in enumerate(f.readlines()):
      for hand, stack in enumerate(line.split(",")):
        data.append({
          "round": round,
          "hand": hand,
          "stack": int(stack),
        })
  
  
  df = pd.DataFrame(data)
  sns.lineplot(x="hand",
               y="stack",
               hue="round",
               data=df,
               palette=sns.color_palette("colorblind"))
  plt.title("Kuhn vs Counter Exploit")
  plt.savefig("data/kuhn__vs__counter_exploit.png")
  


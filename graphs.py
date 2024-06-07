import matplotlib.pyplot as plt
from pandas import read_csv
import os


DATA_DIR = "data/"
GRAPHS_DIR = "graphs/"
SUBSETS = ("put", "get", "remove")
TIMING_MEASURE_RANGE = 100


def main() -> None:
    if not os.path.exists(DATA_DIR):
        print("run C++ program first")
        exit(1)

    if not os.path.exists(GRAPHS_DIR):
        os.mkdir(GRAPHS_DIR)

    for file_name in os.listdir(DATA_DIR):
        csv = read_csv(DATA_DIR + file_name, delimiter=",", index_col=0)
        dataset = file_name.replace(".csv", "")

        for subset in SUBSETS:
            data = csv[csv["op"] == subset].drop(columns=["op"], inplace=False)

            average_times = data.groupby(["map", "users"]).mean()
            average_times["time"] = average_times["time"].map(
                lambda x: x / TIMING_MEASURE_RANGE
            )
            average_times = (
                average_times.reset_index()
                .pivot(index="users", columns="map", values="time")
                .iloc[:-1, :]
            )
            average_times = average_times[["sc", "lp", "qp", "dh", "stl"]]

            shared_title = (
                "of map." + subset + ' for "' + dataset.replace("_", " ") + '"'
            )

            average_times.plot(title="Average time " + shared_title, lw=0.75)
            plt.grid(axis="y")
            plt.ylabel("nanoseconds per op.")
            plt.savefig(GRAPHS_DIR + dataset + "_" + subset + ".png", dpi=300)
            plt.close()

            average_times.mean().plot(kind="bar", title="Average time " + shared_title)
            plt.grid(axis="y")
            plt.ylabel("nanoseconds")
            plt.xticks(rotation="horizontal")
            plt.savefig(GRAPHS_DIR + dataset + "_" + subset + "_bar.png", dpi=300)
            plt.close()

            print("saved", dataset, subset, "graphs")


if __name__ == "__main__":
    main()

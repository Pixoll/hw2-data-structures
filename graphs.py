import matplotlib.pyplot as plt
from os import listdir, mkdir, path
from pandas import read_csv
from shutil import rmtree


DATA_DIR = "data/"
GRAPHS_DIR = "graphs/"
SUBSETS = ("put", "get_(hit)", "get_(miss)", "remove")
TIMING_MEASURE_RANGE = 100


def main() -> None:
    if not path.exists(DATA_DIR):
        print("run C++ program first")
        exit(1)

    if path.exists(GRAPHS_DIR):
        rmtree(GRAPHS_DIR)

    mkdir(GRAPHS_DIR)

    for file_name in listdir(DATA_DIR):
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
                "of map."
                + subset.replace("_", " ")
                + ' for "'
                + dataset.replace("_", " ")
                + '"'
            )

            subset_no_paren = subset.replace("(", "").replace(")", "")

            average_times.plot(title="Average time " + shared_title, lw=0.75)
            plt.grid(axis="y")
            plt.ylabel("nanoseconds per op.")
            plt.savefig(GRAPHS_DIR + dataset + "_" + subset_no_paren + ".png", dpi=300)
            plt.close()

            average_times.mean().plot(kind="bar", title="Average time " + shared_title)
            plt.grid(axis="y")
            plt.ylabel("nanoseconds")
            plt.xticks(rotation="horizontal")
            plt.savefig(GRAPHS_DIR + dataset + "_" + subset_no_paren + "_bar.png", dpi=300)
            plt.close()

            print("saved", dataset, subset, "graphs")


if __name__ == "__main__":
    main()

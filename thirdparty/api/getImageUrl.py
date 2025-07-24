import requests

api_key = "c2b84170f2417adfad3f085206e4a4d2524a651edafba01d1404c5e7a89c583f"

class ImageSearchTool():
    def __init__(self):
        pass

    @staticmethod
    def google_image_search(query="meat", num=5):
        params = {
            "engine": "google",
            "q": query,
            "tbm": "isch",
            "num": num,
            "api_key": api_key
        }

        response = requests.get("https://serpapi.com/search", params=params)
        data = response.json()

        links = []
        for image in data.get("images_results", []):
            links.append(image["original"])
            if len(links) >= num:
                break

        return links

# Sử dụng
def main():
    query = input("input food: ") 

    results = ImageSearchTool.google_image_search(query)

    for idx, url in enumerate(results, 1):
        print(f"{idx}. {url}")

if __name__ == "__main__":
    main()
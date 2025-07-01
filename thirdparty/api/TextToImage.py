# Thư viện gửi yêu cầu HTTP
import requests

# Khóa API từ SerpAPI (dùng để truy cập Google Search API)
api_key = "c2b84170f2417adfad3f085206e4a4d2524a651edafba01d1404c5e7a89c583f"

# Hàm tìm kiếm hình ảnh trên Google 
def google_image_search(query="meat", num=5):
    # Thiết lập tham số truy vấn
    params = {
        "engine": "google",     # Công cụ tìm kiếm là Google
        "q": query,             # Từ khóa tìm kiếm
        "tbm": "isch",          # Tìm hình ảnh (image search)
        "num": num,             # Số lượng hình ảnh cần lấy
        "api_key": api_key      # API key để xác thực với SerpAPI
    }

    # Gửi yêu cầu GET tới SerpAPI
    response = requests.get("https://serpapi.com/search", params=params)
    data = response.json()  # Chuyển đổi kết quả thành định dạng JSON

    # Trích xuất danh sách URL ảnh gốc
    links = []
    for image in data.get("images_results", []):
        links.append(image["original"])  # Lấy link ảnh gốc
        if len(links) >= num:
            break

    return links

# Hàm main chạy chương trình
def main():
    query = input("input food: ")  # Nhập từ khóa tìm kiếm (ví dụ: thịt, rau, gạo...)

    results = google_image_search(query)  # Tìm ảnh tương ứng

    # In ra các link hình ảnh
    for idx, url in enumerate(results, 1):
        print(f"{idx}. {url}")

if __name__ == "__main__":
    main()

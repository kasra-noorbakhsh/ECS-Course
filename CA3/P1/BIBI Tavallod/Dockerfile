FROM golang:1.24-alpine AS builder

WORKDIR /app

COPY . .

RUN apk add --no-cache upx && \
    go build -ldflags="-s -w" -o main && \
    upx --best --lzma -q main

FROM scratch

COPY --from=builder /app/main /main

EXPOSE 80

ENTRYPOINT ["/main"]

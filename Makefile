
ans: ans.cpp ans2.cpp glad.c
	g++ -o ans ans.cpp glad.c -lGL -lglfw -ldl
	g++ -o ans2 ans2.cpp glad.c -lGL -lglfw -ldl

clean:
	rm ans
	rm ans2

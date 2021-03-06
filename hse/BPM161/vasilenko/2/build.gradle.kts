import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

/*
 * This file was generated by the Gradle 'init' task.
 *
 * This generated file contains a sample Kotlin library project to get you started.
 */
plugins {
    // Apply the Kotlin JVM plugin to add support for Kotlin on the JVM.
    id("org.jetbrains.kotlin.jvm").version("1.3.20")
    id("com.github.erizo.gradle.jcstress").version("0.8.1-SNAPSHOT")
}

repositories {
    // Use jcenter for resolving your dependencies.
    // You can declare any Maven/Ivy/file repository here.
    jcenter()

    mavenCentral()
    maven(url="https://dl.bintray.com/devexperts/Maven")
}

dependencies {
    // Use the Kotlin JDK 8 standard library.
    compile("org.jetbrains.kotlin:kotlin-stdlib-jdk8")

    // Use the Kotlin test library.
    testCompile("org.jetbrains.kotlin:kotlin-test")

    // Use the Kotlin JUnit integration.
    testCompile("org.jetbrains.kotlin:kotlin-test-junit")

    testCompile("com.devexperts.lincheck:lincheck:2.0")

    testCompile("org.hamcrest:hamcrest-junit:2.0.0.0")
}

jcstress {
    jcstressDependency="org.openjdk.jcstress:jcstress-core:0.5"
}